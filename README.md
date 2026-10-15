# Braxon Studio and TAC-W Firmware

The working Braxon Studio web configurator and custom ZMK firmware for the
TAC-W nRF52840 macropad: 12 keys, two rotary encoders, four WS2812 LEDs,
and a 240 x 135 ST7789 display.

## Repository Contents

| Location | Contents |
| --- | --- |
| `index.html`, `app.js`, `styles.css`, `assets/` | Current static web configurator |
| `server.py` | Optional local server with the existing Windows BLE bridge |
| `firmware/config/` | Keymap, hardware overlays, display UI, animation, logo, and remapping RPC |
| `firmware/patches/tac-w-zmk.patch` | Required changes to upstream ZMK |
| `firmware/uf2/` | Current firmware image and the earlier commercial backup |
| `firmware/build.ps1` | Firmware build and patch script |
| `west.yml` | Pinned ZMK, Zephyr, and module revisions |

## Run the Website

From the repository folder:

```sh
python -m http.server 8765 --bind 127.0.0.1
```

Open <http://127.0.0.1:8765/> in a browser that supports the chosen transport.
USB configuration uses Web Serial. Bluetooth configuration uses Web Bluetooth
and the firmware's Nordic UART Service; browser and OS support varies.

For the optional existing Windows BLE bridge, run `python server.py` instead.
It listens on the same loopback address and port. The bridge requires the
Python WinRT bindings imported at the top of `server.py`. Static hosting
does not provide these local Windows endpoints.

## Host the Website

The web UI is static and has no Node.js build or install step. For Vercel,
select the **Other** framework and serve the repository root. Use HTTPS for
browser device access. GitHub Pages can also serve the root static files.

Application and website launchers send keyboard macros. Application launchers
accept a manually entered path or command.

## Firmware

See [firmware/README.md](firmware/README.md) for the exact board, current pin
assignments, bootloader layout, and build instructions. The internal shield
identifier remains `wire_x` because the existing source and stored settings
use it; the visible device name is `TAC-W`.

The supplied images use a custom no-SoftDevice memory layout. Check the
firmware notes before flashing a different board or bootloader.
