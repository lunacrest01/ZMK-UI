import asyncio
import json
import os
import queue
import re
import subprocess
import threading
import time
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

try:
    from winrt.windows.devices.bluetooth import BluetoothLEDevice
    from winrt.windows.devices.bluetooth.genericattributeprofile import (
        GattClientCharacteristicConfigurationDescriptorValue,
        GattWriteOption,
    )
    from winrt.windows.storage.streams import DataReader, DataWriter

    WINRT_BLE_AVAILABLE = True
except ImportError:
    WINRT_BLE_AVAILABLE = False


HOST = "127.0.0.1"
PORT = 8765
ROOT = Path(__file__).resolve().parent
LAUNCHER_ROOT = Path(os.environ.get("PUBLIC", r"C:\Users\Public")) / "TACW"
NUS_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
NUS_RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
NUS_TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
BLE_CHUNK_SIZE = 20


def json_bytes(payload):
    return json.dumps(payload, ensure_ascii=False).encode("utf-8")


def choose_windows_app():
    if os.name != "nt":
        raise RuntimeError("Installed-app selection currently requires Windows")

    script = r"""
Add-Type -AssemblyName System.Windows.Forms
$owner = New-Object System.Windows.Forms.Form
$owner.TopMost = $true
$owner.ShowInTaskbar = $false
$owner.StartPosition = 'CenterScreen'
$owner.Size = New-Object System.Drawing.Size(1, 1)
$dialog = New-Object System.Windows.Forms.OpenFileDialog
$dialog.Title = 'Choose an application for TAC-W'
$dialog.Filter = 'Applications and shortcuts (*.exe;*.lnk;*.bat;*.cmd)|*.exe;*.lnk;*.bat;*.cmd|All files (*.*)|*.*'
$dialog.CheckFileExists = $true
$dialog.Multiselect = $false
$result = $dialog.ShowDialog($owner)
$owner.Close()
if ($result -eq [System.Windows.Forms.DialogResult]::OK) {
    [Console]::OutputEncoding = [System.Text.Encoding]::UTF8
    Write-Output $dialog.FileName
}
"""
    creation_flags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    result = subprocess.run(
        ["powershell.exe", "-NoProfile", "-STA", "-Command", script],
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        creationflags=creation_flags,
        timeout=180,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or "Windows application picker failed")
    selected = result.stdout.strip()
    return Path(selected) if selected else None


def create_launcher(slot, target):
    LAUNCHER_ROOT.mkdir(parents=True, exist_ok=True)
    launcher = LAUNCHER_ROOT / f"a{slot}.vbs"
    quoted_target = '"""' + str(target).replace('"', '""') + '"""'
    launcher.write_text(
        "Set tacwShell = CreateObject(\"WScript.Shell\")\r\n"
        f"tacwShell.Run {quoted_target}, 1, False\r\n",
        encoding="utf-16",
    )
    return launcher


def find_tacw_bluetooth_address():
    script = r"""
$device = Get-PnpDevice -Class Bluetooth -ErrorAction SilentlyContinue |
    Where-Object { $_.FriendlyName -eq 'TAC-W' -and $_.InstanceId -like 'BTHLE\DEV_*' } |
    Select-Object -First 1 -ExpandProperty InstanceId
if ($device) { Write-Output $device }
"""
    creation_flags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    result = subprocess.run(
        ["powershell.exe", "-NoProfile", "-Command", script],
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        creationflags=creation_flags,
        timeout=15,
        check=False,
    )
    match = re.search(r"BTHLE\\DEV_([0-9A-F]{12})", result.stdout, re.IGNORECASE)
    if not match:
        raise RuntimeError("TAC-W is not paired in Windows Bluetooth settings")
    return int(match.group(1), 16)


class WindowsBleBridge:
    def __init__(self):
        self.loop = asyncio.new_event_loop()
        self.thread = threading.Thread(target=self._run_loop, name="tacw-ble", daemon=True)
        self.thread.start()
        self.transaction_lock = threading.Lock()
        self.lines = queue.Queue()
        self.buffer = bytearray()
        self.buffer_lock = threading.Lock()
        self.device = None
        self.rx = None
        self.tx = None
        self.tx_token = None

    def _run_loop(self):
        asyncio.set_event_loop(self.loop)
        self.loop.run_forever()

    def _submit(self, coroutine, timeout=25):
        future = asyncio.run_coroutine_threadsafe(coroutine, self.loop)
        return future.result(timeout=timeout)

    def connect(self):
        with self.transaction_lock:
            if self.device is not None and self.rx is not None and self.tx is not None:
                return {"name": self.device.name or "TAC-W", "transport": "windows-ble"}

            address = find_tacw_bluetooth_address()
            last_error = None
            for attempt in range(3):
                try:
                    self._submit(self._connect(address))
                    break
                except Exception as error:
                    last_error = error
                    try:
                        self._submit(self._disconnect())
                    except Exception:
                        pass
                    if attempt < 2:
                        time.sleep(0.7)
            else:
                raise last_error
        return {"name": self.device.name or "TAC-W", "transport": "windows-ble"}

    async def _connect(self, address):
        await self._disconnect()
        self._clear_received()

        device = await BluetoothLEDevice.from_bluetooth_address_async(address)
        if device is None:
            raise RuntimeError("Windows could not open TAC-W over Bluetooth")

        services_result = await device.get_gatt_services_async()
        if int(services_result.status) != 0:
            device.close()
            raise RuntimeError("TAC-W Bluetooth services are unavailable")

        service = next(
            (service for service in services_result.services if str(service.uuid).lower() == NUS_SERVICE_UUID),
            None,
        )
        if service is None:
            device.close()
            raise RuntimeError("TAC-W remapping service was not found")

        characteristics_result = await service.get_characteristics_async()
        if int(characteristics_result.status) != 0:
            device.close()
            raise RuntimeError("TAC-W remapping characteristics are unavailable")

        rx = next(
            (
                characteristic
                for characteristic in characteristics_result.characteristics
                if str(characteristic.uuid).lower() == NUS_RX_UUID
            ),
            None,
        )
        tx = next(
            (
                characteristic
                for characteristic in characteristics_result.characteristics
                if str(characteristic.uuid).lower() == NUS_TX_UUID
            ),
            None,
        )
        if rx is None or tx is None:
            device.close()
            raise RuntimeError("TAC-W remapping channel is incomplete")

        token = tx.add_value_changed(self._on_value_changed)
        notify_status = await tx.write_client_characteristic_configuration_descriptor_async(
            GattClientCharacteristicConfigurationDescriptorValue.NOTIFY
        )
        if int(notify_status) != 0:
            tx.remove_value_changed(token)
            device.close()
            raise RuntimeError("Windows could not enable TAC-W BLE notifications")

        self.device = device
        self.rx = rx
        self.tx = tx
        self.tx_token = token

    def _on_value_changed(self, sender, args):
        reader = DataReader.from_buffer(args.characteristic_value)
        data = bytearray(reader.unconsumed_buffer_length)
        reader.read_bytes(data)

        with self.buffer_lock:
            self.buffer.extend(data)
            while b"\n" in self.buffer:
                raw, _, remainder = self.buffer.partition(b"\n")
                self.buffer[:] = remainder
                self.lines.put(raw.decode("utf-8", errors="replace"))

    def _clear_received(self):
        with self.buffer_lock:
            self.buffer.clear()
        while True:
            try:
                self.lines.get_nowait()
            except queue.Empty:
                break

    def write_and_wait(self, text):
        if not isinstance(text, str) or not text.strip():
            raise ValueError("A BLE command is required")

        try:
            expected_id = json.loads(text.splitlines()[0]).get("id")
        except (json.JSONDecodeError, AttributeError):
            expected_id = None

        with self.transaction_lock:
            if self.device is None or self.rx is None:
                address = find_tacw_bluetooth_address()
                self._submit(self._connect(address))

            self._clear_received()
            try:
                self._submit(self._write(text.encode("utf-8")))
            except Exception:
                address = find_tacw_bluetooth_address()
                self._submit(self._connect(address))
                self._clear_received()
                self._submit(self._write(text.encode("utf-8")))

            received = []
            deadline = time.monotonic() + 18
            while time.monotonic() < deadline:
                try:
                    line = self.lines.get(timeout=max(0.05, deadline - time.monotonic()))
                except queue.Empty:
                    break
                received.append(line)
                try:
                    reply = json.loads(line)
                except json.JSONDecodeError:
                    continue
                if expected_id is None or reply.get("id") == expected_id:
                    return "\n".join(received) + "\n"

        raise TimeoutError("TAC-W did not reply over BLE")

    async def _write(self, data):
        if self.rx is None:
            raise RuntimeError("TAC-W BLE is not connected")

        for offset in range(0, len(data), BLE_CHUNK_SIZE):
            writer = DataWriter()
            writer.write_bytes(data[offset : offset + BLE_CHUNK_SIZE])
            status = await self.rx.write_value_with_option_async(
                writer.detach_buffer(),
                GattWriteOption.WRITE_WITHOUT_RESPONSE,
            )
            if int(status) != 0:
                raise RuntimeError("TAC-W BLE write failed")
            await asyncio.sleep(0.01)

    def disconnect(self):
        with self.transaction_lock:
            self._submit(self._disconnect())

    async def _disconnect(self):
        if self.tx is not None and self.tx_token is not None:
            try:
                self.tx.remove_value_changed(self.tx_token)
            except Exception:
                pass
        if self.device is not None:
            try:
                self.device.close()
            except Exception:
                pass

        self.device = None
        self.rx = None
        self.tx = None
        self.tx_token = None
        self._clear_received()


BLE_BRIDGE = WindowsBleBridge() if os.name == "nt" and WINRT_BLE_AVAILABLE else None


class BraxonStudioHandler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(ROOT), **kwargs)

    def send_json(self, status, payload):
        body = json_bytes(payload)
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        if self.path not in {
            "/api/select-app",
            "/api/ble/connect",
            "/api/ble/write",
            "/api/ble/disconnect",
        }:
            self.send_json(404, {"error": "Unknown endpoint"})
            return

        try:
            length = min(int(self.headers.get("Content-Length", "0")), 65536)
            request = json.loads(self.rfile.read(length) or b"{}")

            if self.path == "/api/ble/connect":
                if BLE_BRIDGE is None:
                    raise RuntimeError("Use Web Bluetooth for BLE remapping on this operating system")
                result = BLE_BRIDGE.connect()
                self.send_json(200, {"ok": True, **result})
                return

            if self.path == "/api/ble/write":
                if BLE_BRIDGE is None:
                    raise RuntimeError("The Windows BLE bridge is unavailable")
                text = BLE_BRIDGE.write_and_wait(request.get("text"))
                self.send_json(200, {"ok": True, "text": text})
                return

            if self.path == "/api/ble/disconnect":
                if BLE_BRIDGE is not None:
                    BLE_BRIDGE.disconnect()
                self.send_json(200, {"ok": True})
                return

            slot = int(request.get("slot", 0))
            if slot < 0 or slot > 59:
                raise ValueError("Invalid macro slot")

            selected = choose_windows_app()
            if selected is None:
                self.send_json(200, {"cancelled": True})
                return
            if not selected.exists():
                raise FileNotFoundError("The selected application no longer exists")

            create_launcher(slot, selected)
            self.send_json(
                200,
                {
                    "cancelled": False,
                    "name": selected.stem,
                    "command": f"wscript %public%\\tacw\\a{slot}.vbs",
                },
            )
        except Exception as error:
            self.send_json(500, {"error": str(error)})

    def log_message(self, fmt, *args):
        print(f"[Braxon Studio] {self.address_string()} - {fmt % args}")


if __name__ == "__main__":
    os.chdir(ROOT)
    server = ThreadingHTTPServer((HOST, PORT), BraxonStudioHandler)
    print(f"Braxon Studio running at http://{HOST}:{PORT}/")
    server.serve_forever()
