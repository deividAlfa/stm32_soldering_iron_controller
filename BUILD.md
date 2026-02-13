# Building the STM32 Soldering Iron Controller

## TL;DR (Docker)

```bash
# Build release matrix (default) -> .bin + .zip in ./build
docker compose up

# Build a specific target
BOARD=KSGER_v3 DISPLAY=SSD1306 docker compose up

# Artifacts appear in ./build/
ls build/*
```

## Supported Boards

| BOARD         | MCU           | DISPLAY options     | Status |
|---------------|---------------|---------------------|--------|
| `KSGER_v1.5`  | STM32F103     | `SSD1306`, `ST7565` | Implemented |
| `KSGER_v2`    | STM32F101     | `SSD1306`           | Implemented |
| `KSGER_v3`    | STM32F101     | `SSD1306`, `ST7565` | Implemented |
| `Quicko_072`  | STM32F072     | `SSD1306`, `ST7565` | Implemented |
| `Quicko_103`  | STM32F103     | `SSD1306`, `ST7565` | Implemented |
| `T12_958_v2`  | STM32F103     | `SSD1306`, `ST7565` | Implemented |

## Docker Build (Recommended)

The Docker image bundles the ARM toolchain and ST HAL/CMSIS libraries
so you don't need to install anything locally except Docker.

```bash
# First build (pulls deps, takes a few minutes, cached afterwards)
docker compose build

# Build release matrix (default)
docker compose up

# Build one target
BOARD=Quicko_072 DISPLAY=ST7565 docker compose up

# Build all valid board/display combinations from the legacy Windows flow
BOARD=all docker compose up

# Explicit release mode (same as default)
BOARD=release docker compose up
```

`BOARD=release` outputs these zip names in `./build/`:

- `KSGER_v1_5_LCD_ST7565.zip`
- `KSGER_v1_5_OLED.zip`
- `KSGER_v2_OLED.zip`
- `KSGER_v3_LCD_ST7565.zip`
- `KSGER_v3_OLED.zip`
- `Quecoo_T12-958_v2_LCD_ST7565.zip`
- `Quecoo_T12-958_v2_OLED.zip`
- `Quicko_STM32F072_LCD_ST7565.zip`
- `Quicko_STM32F072_OLED.zip`
- `Quicko_STM32F103_LCD_ST7565.zip`

The container runs the build and exits. Binaries and zip files land in `./build/`.
Both `DISPLAY` and `OLED` environment variables are accepted for display selection.

## Windows Build (Original Method)

Requires [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
installed in `C:\ST\`.

```
Building_script.bat 4 3
```

See `Building_script.bat` for the interactive menu and profile list.

## Manual Build (Linux/macOS)

Requires `arm-none-eabi-gcc` and the STM32CubeF1 HAL package.

```bash
# Install toolchain (macOS)
brew install --cask gcc-arm-embedded

# Install toolchain + zip (Debian/Ubuntu)
sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi zip

# Run the build script (fetches HAL/CMSIS on first run)
./scripts/docker-build.sh
```

The build script auto-detects whether it's running inside Docker or natively
and fetches HAL/CMSIS to `/tmp/STM32CubeF1` if needed.

## Why Not Just `make`?

The project was designed for STM32CubeMX, which generates ~5 board-specific
source files at build time from `.ioc` hardware descriptions. These files
(peripheral init, GPIO mapping, clock config, HAL module selection) are not
checked into the repo — they're in `.gitignore`.

The `Makefile` in the repo is a convenience shortcut for KSGER v3 SSD1306
only. For other boards or a reproducible build, use Docker or the build
script which generates the CubeMX-equivalent files automatically.
