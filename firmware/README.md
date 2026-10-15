# TAC-W Firmware

This directory contains the current custom ZMK source configuration and
existing UF2 images. The firmware has not been changed to add features for
this repository import.

## Images

| File | Origin |
| --- | --- |
| `uf2/TAC-W.uf2` | Current source build; uses the pin assignments below |
| `uf2/TAC-W_COMMERCIAL_V2_BACKUP.uf2` | Preserved commercial image from June 14, 2026; uses earlier wiring |
| `uf2/SHA256SUMS` | SHA-256 checksums of the supplied images |

The backup is retained as an earlier artifact. It is not interchangeable
with the current image when the physical wiring differs.

Target board: `promicro_nrf52840/nrf52840/uf2`.
Internal shield name: `wire_x`. Advertised keyboard name: `TAC-W`.

`config/boards/nicenano_nosd.overlay` expects the existing custom UF2 bootloader
without a SoftDevice: application at `0x1000`, settings at `0xEC000` (32 KiB),
and bootloader at `0xF4000`. These images are specific to that layout.

## Current Source Pin Assignments

These values are taken from the supplied overlays. They describe the current
firmware, rather than a general pinout for all TAC-W revisions.

| Component | Signal | nRF52840 GPIO |
| --- | --- | --- |
| Matrix | Rows 0, 1, 2, 3 | P0.06, P0.08, P1.09, P0.07 |
| Matrix | Columns 0, 1, 2, 3 | P0.20, P0.22, P0.24, P1.00 |
| Encoder 0 | A, B | P0.09, P1.11 |
| Encoder 1 | A, B | P1.04, P1.06 |
| WS2812 strip | Data, four LEDs | P0.10 |
| ST7789 | SCK, MOSI | P0.12, P1.02 |
| ST7789 | CS, DC, reset | P1.13, P0.31, P0.29 |
| Display power | MOSFET control | P1.07 |
| Battery divider | ADC AIN0 | P0.02 |

Matrix diodes are `col2row`. The encoder push switches occupy matrix positions
0.0 and 0.1. The divider configuration is 1 Mohm above and 1 Mohm below the ADC
input. Check the physical board against these assignments before flashing.

## Build from a Fresh Checkout

Install Git, Python, west, CMake, Ninja, PowerShell, and a suitable Zephyr SDK.
The existing local build uses Zephyr SDK 0.16.8. All upstream source revisions
are pinned in the root `west.yml`.

Create a dedicated workspace. From its empty parent folder:

```powershell
git clone https://github.com/lunacrest01/ZMK-UI.git ZMK-UI
west init -l ZMK-UI
west update
west zephyr-export
python -m pip install -r zephyr/scripts/requirements.txt
pwsh -File ./ZMK-UI/firmware/build.ps1
```

The script verifies the pinned ZMK commit, applies the required patch if it
is not already applied, builds the configuration, and writes the result to
`uf2/TAC-W.uf2`. A newly generated checksum is written to `uf2/SHA256SUMS.local`.
Build intermediates stay in `.build/`, which Git ignores.

An existing west workspace can also be used:

```powershell
pwsh -File ./firmware/build.ps1 -Workspace C:/path/to/workspace -ZmkSource C:/path/to/workspace/zmk
```

`-West` accepts a path to a west executable. The script does not flash hardware.

## Required ZMK Patch

`patches/tac-w-zmk.patch` preserves the local changes to:

- Advertise the remapping NUS service and support timed BLE advertising windows.
- Remap encoder sensor bindings at runtime.
- Average battery ADC samples and use the existing battery percentage curve.

The custom RPC, persistence, macros, RGB effects, power timeouts, display
screens, and display assets live in `config/boards/shields/wire_x/`.
Upstream licenses and copyright notices remain with their respective projects.
