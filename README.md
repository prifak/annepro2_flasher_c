# annepro2_flasher_c

This is a lightweight CLI utility to flash the Anne Pro 2 keyboard. 
Originally rewritten from Rust to clean, dependency-free C. Now fully cross-platform (Windows, macOS, Linux) powered by CMake.

---

## Download Pre-built Binaries

You don't need to build this from source! Ready-to-use binaries for **Windows, macOS, and Linux** are automatically generated.
Check the [Releases](https://github.com/prifak/annepro2-tools-c/releases) page or the GitHub Actions tab to download the latest `.exe` or executable for your system.

---

## Build Instructions (From Source)

### Prerequisites

* **All platforms:** `cmake` (version 3.14+) and a C compiler (`gcc`, `clang`, or MSVC).
* **Linux only:** You need `pkg-config` and `libudev-dev` to compile the USB backend.
  * *Ubuntu/Debian:* `sudo apt install cmake gcc pkg-config libudev-dev`
  * *Arch/Manjaro:* `sudo pacman -S cmake gcc pkgconf systemd`
  * *Fedora:* `sudo dnf install cmake gcc pkgconf-pkg-config systemd-devel`

### Building

1. Clone the repository:
   ```bash
   git clone [https://github.com/prifak/annepro2-tools-c.git](https://github.com/prifak/annepro2-tools-c.git)
   cd annepro2-tools-c
   ```

2. Configure the project and generate build files:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   ```

3. Compile the code:
   ```bash
   cmake --build build --config Release
   ```

The compiled executable will be located inside the build/ directory (e.g., build/annepro2_flasher or build/bin/annepro2_flasher depending on your CMake configuration).

## Usage
You can run the executable directly from the build folder. If run without arguments, the tool displays a help message with all available options.

    Important: The tool will wait up to 10 seconds for the keyboard to enter IAP (flash) mode.
    To do this: unplug the keyboard, hold the ESC key, then plug it back in.

### To flash main firmware:
  ```bash
  ./build/annepro2_flasher ./my_firmware.bin
  ```


### To flash the LED Processor:
  ```bash
  ./build/annepro2_flasher -t led ./my_led_firmware.bin
  ```


### To flash the Bluetooth (BLE) firmware:
  ```bash
  ./build/annepro2_flasher -t ble ./my_ble_firmware.bin
  ```

### Auto-reboot
If you want the keyboard to automatically reboot after a successful flash, just add the --boot flag:
```bash
./build/annepro2_flasher --boot ./my_firmware.bin
```

## Disclaimer
I am not responsible for bricked or broken keyboards. Use this program at your own risk.
