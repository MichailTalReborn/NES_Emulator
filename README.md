# NES_Emulator

A Nintendo Entertainment System emulator written in C++, built upon the excellent YouTube tutorial series by **OneLoneCoder (javidx9)**. This project follows and extends that series while adding features such as a full CLI interface, debug panels, and audio visualisation.

---

## Credits & Acknowledgements

This project is based on the **NES Emulator** series by [OneLoneCoder (javidx9)](https://www.youtube.com/@javidx9) on YouTube.

- 📺 [NES Emulator from Scratch playlist](https://www.youtube.com/playlist?list=PLrOv9FMX8xJHqMvSGB_9G9nZZ_4IgteYf)
- 🌐 [OneLoneCoder website](https://onelonecoder.com)
- 💻 [OneLoneCoder GitHub](https://github.com/OneLoneCoder)

The following libraries by OneLoneCoder are used and included in this project:

| Library | Purpose |
|---|---|
| `olcPixelGameEngine.h` | Windowing, rendering and input |
| `olcPGEX_Sound.h` | Audio output (ALSA on Linux) |

Both are used under the [OLC-3 License](https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/LICENCE.md).

---

## Features

- **MOS 6502 CPU** emulation
- **2C02 PPU** (Picture Processing Unit) with accurate rendering
- **2A03 APU** (Audio Processing Unit) — Pulse 1 & 2, Noise channel
- **iNES ROM format** support
- **Mapper support:** 000, 001, 002, 003, 004, 066
- **CLI interface** with optional debug panels
- Real-time **audio-synced emulation**

---

## Requirements

### Linux
```bash
sudo dnf install gcc-c++ mesa-libGL-devel libpng-devel libX11-devel alsa-lib-devel   # Fedora/Bazzite
sudo apt install build-essential libgl1-mesa-dev libpng-dev libx11-dev libasound2-dev # Debian/Ubuntu
```

> **Bazzite / Fedora Atomic users:** Use [Toolbox](https://containertoolbx.org/) or [Distrobox](https://distrobox.it/) to install development packages without layering onto the immutable base image:
> ```bash
> toolbox enter
> sudo dnf install gcc-c++ mesa-libGL-devel libpng-devel libX11-devel alsa-lib-devel
> ```

### Compiler
- GCC or Clang with **C++17** support

---

## Building

```bash
git clone https://github.com/MichailTalReborn/NES_Emulator.git
cd NES_Emulator
make
```

Additional build targets:

```bash
make debug    # Build with debug symbols and no optimisation (-g -O0)
make clean    # Remove build artefacts
```

New `.cpp` files placed in `src/` are picked up automatically — no Makefile changes needed.

---

## Usage

```bash
./NES_Emulator <path-to-rom.nes> [options]
```

### Options

| Flag | Description |
|---|---|
| `--cpu` | Show CPU register status panel |
| `--code` | Show disassembled code around the program counter |
| `--ram` | Show zero-page and PRG RAM hex dumps |
| `--audio` | Show APU channel waveform visualisers |
| `-h`, `--help` | Print help and exit |

When **no debug flags** are given the emulator runs in clean full-window mode (512×480). Any debug flag enables the side panel (780×480).

### Examples

```bash
# Play a game — full window, no panels
./NES_Emulator roms/supermario.nes

# Debug session with CPU state and disassembly
./NES_Emulator roms/supermario.nes --cpu --code

# Everything at once
./NES_Emulator roms/supermario.nes --cpu --code --ram --audio
```

---

## Controls

| Key | Action |
|---|---|
| Arrow keys | D-Pad |
| `X` | A button |
| `Z` | B button |
| `A` | Select |
| `S` | Start |
| `R` | Reset |
| `P` | Cycle debug palette |

### Debug controls *(only active with at least one debug panel enabled)*

| Key | Action |
|---|---|
| `Space` | Pause / resume emulation |
| `C` | Step one CPU instruction *(while paused)* |
| `F` | Step one full frame *(while paused)* |

---

## Project Structure

```
NES_Emulator/
├── src/
│   ├── main.cpp              # Entry point, CLI parsing, PGE application
│   ├── Bus.cpp / .h          # Main system bus
│   ├── olc6502.cpp / .h      # MOS 6502 CPU
│   ├── olc2C02.cpp / .h      # PPU
│   ├── olc2A03.cpp / .h      # APU
│   ├── Cartridge.cpp / .h    # iNES ROM loader
│   ├── Mapper.cpp / .h       # Mapper base class
│   ├── Mapper_000.cpp / .h   # NROM
│   ├── Mapper_001.cpp / .h   # MMC1
│   ├── Mapper_002.cpp / .h   # UxROM
│   ├── Mapper_003.cpp / .h   # CNROM
│   ├── Mapper_004.cpp / .h   # MMC3
│   ├── Mapper_066.cpp / .h   # GxROM
│   ├── olcPixelGameEngine.h  # OLC rendering engine
│   └── olcPGEX_Sound.h       # OLC sound extension
└── Makefile
```

---

## Known Issues & Patches

### `olcPGEX_Sound.h` — Linux ALSA audio produces silence

The Linux ALSA audio thread in the version of `olcPGEX_Sound.h` bundled with this project contains a bug where the comma operator causes all audio samples to be silently discarded, resulting in a constant DC signal. A patch is applied in this repo.

**Root cause:** Missing `clip()` call in the Linux sample write loop (present correctly in the Windows implementation). See the [upstream issue](https://github.com/OneLoneCoder/olcPixelGameEngine) for details.

---

## License

This project is for **educational purposes**, following the OneLoneCoder NES series.  
`olcPixelGameEngine.h` and `olcPGEX_Sound.h` are © OneLoneCoder, used under the [OLC-3 License](https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/LICENCE.md).

Nintendo, NES, and related trademarks are the property of Nintendo Co., Ltd. This project is not affiliated with or endorsed by Nintendo.
