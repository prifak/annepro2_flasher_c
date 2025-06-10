# annepro2_flasher_c
This is cli utility for flash annepro2 keyboard, rewriten from existing one (from Rust to clear C)

---

## Dependencies

To build this project externally, you only need **hidapi** — specifically the *hidapi-hidraw* variant (header files and linkable library).

> Note:  
> - In most cases, `hidapi-hidraw` is used by default.  
> - If you specifically need `hidapi-libusb`, package names may differ.  
> - On Debian 11+ and Ubuntu 20.04+, `libhidapi-dev` defaults to `hidraw`.

### Installing `hidapi` development package

| Distribution         | Install Command                                                                 |
|----------------------|----------------------------------------------------------------------------------|
| **Debian/Ubuntu**    | `sudo apt install libhidapi-dev`                                                |
| **Fedora**           | `sudo dnf install hidapi-devel`                                                 |
| **CentOS/RHEL**      | `sudo yum install hidapi-devel`                                                 |
| **Arch Linux/Manjaro** | `sudo pacman -S hidapi`                                                       |
| **Gentoo**           | `sudo emerge dev-libs/hidapi`                                                   |
| **Void Linux**       | `sudo xbps-install -S libhidapi-devel`                                          |
| **Nix/NixOS**        | `nix-shell -p hidapi` *(or add `hidapi` to `buildInputs`)*                      |
| **Alpine Linux**     | `sudo apk add hidapi-dev`                                                       |
| **Slackware**        | via SlackBuilds or sbopkg: `sbopkg -i hidapi`                                   |

<details>
<summary>Example `shell.nix` for Nix/NixOS</summary>

```nix
{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = [ pkgs.hidapi ];
}
```
</details>

## Build instructions
  1. Install dependencies
  2. Do this in your terminal

    git clone https://github.com/prifak/annepro2-tools-c.git
    cd annepro2-tools-c
    make
    sudo make install

## Usage 
To flash firmware:
```bash
annepro2_flasher_c ./my_firmware.bin
```

To flash light processor:
```bash
annepro2_flasher_c -t led ./my_led_firmware.bin
```

To flash bluetooth firmware:
```bash
annepro2_flasher_c -t ble ./my_ble_firmware.bin
```
If you want auto reboot keyboard after flash just add ```--boot``` parameter:
```bash
annepro2_flasher_c --boot ./my_firmware.bin
```
If run without arguments, the tool displays a help message with all available options.

> The tool will wait up to 10 seconds for the keyboard to enter IAP mode.
> To do this: unplug the keyboard, hold ESC, then plug it back in.

## P.S.
  I am not responsible for broken keyboards. Use this program at your own risk.
    
