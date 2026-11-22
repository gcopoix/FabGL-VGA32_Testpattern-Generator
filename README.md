# Testimage generator for LilyGO VGA32 board

This project is a VGA Mode test pattern generator for the [LilyGO VGA32](https://lilygo.cc/en-us/products/fabgl-vga32) board, using the [FabGL library of Fabrizio Di Vittorio](https://github.com/fdivitto/fabgl).

<img src="img/VGA32_persp.png" alt="Alt text" width="25%"> <img src="img/testimage.jpeg" alt="Alt text" width="30%">



It is based on [ModelineStudio](https://github.com/fdivitto/FabGL/tree/master/examples/VGA/ModelineStudio) sample of the [FabGL](https://github.com/fdivitto/FabGL) library with some additional features:
- typical retro TV-like test image generation
- all [supported VGA Modes](https://github.com/fdivitto/FabGL/blob/master/src/fabglconf.h#L172-L324)
- all modes in 64 (!) colors (implemented using [VGADirectController](https://github.com/fdivitto/FabGL/blob/master/src/dispdrivers/vgadirectcontroller.h#L77), using minimal RAM ressources)
- next Video mode selection on PROG Button press
- transferred to [PlatformIO](https://platformio.org)
- [VisusalStudio Code](https://code.visualstudio.com) setup using [PlatformIO IDE](https://github.com/platformio/platformio-vscode-ide) extension


## Setup

### with VisualStudio Code

Clone this repo (or unpack the compressed folder) and open it by one of these methods:
- open the project by double-click on [vs.code-workspace](vs.code-workspace) workspace file
- open a terminal and enter
   ```
   code <path to the folder of this project>
   ```
- right-click on this folder and select `Open with VisualStudio Code` from the context menu

This will
1. Open VisualStudio Code
2. Install the required PlatformIO [extension](.vscode/extensions.json#L5)
3. Setup PlatformIO with the [required library](platformio.ini#L21-L22)

### from Console

Typical commandline sequence to setup platformio on your host system:
```
# get installer script (see https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html)
wget https://github.com/platformio/platformio-core-installer/raw/develop/get-platformio.py

# run installer script with python and add platformio core folder to PATH
python3 get-platformio.py
PATH=$PATH:$HOME/.platformio/penv/bin

# fetch this repository and enter the cloned folder
git clone <url of this repo>
cd <cloned repo folder>
```

## Build
Simply build the project by one of these options:
- click on the `PlatformIO::Build` icon in the bottom status line of VisualStudio Code
- press `Ctrl-Shift-B`
- select `Run Build Task` from the `Terminal` menu
- select `Run Task` from the `Terminal` menu and select `PlatformIO::Build`
- open a terminal (e.g. by selecting `New Terminal` from the `Terminal` menu) and enter
  ```
  pio run
  ```
  If this is executed the first time (in a shell environment only) PlatformIO will automatically fetch all required depoendencies.

## Flash
Make sure you have the VGA32 board connected via USB cable to your host computer and the [udev rules](https://docs.platformio.org/en/stable/core/installation/udev-rules.html) are installed.

Simply flash the project by one of these options:
- click on the `PlatformIO::Upload` icon in the bottom status line of VisualStudio Code
- select `Run Task` from the `Terminal` menu and select `PlatformIO::Upload`
- open a terminal (e.g. by selecting `New Terminal` from the `Terminal` menu) and enter
  ```
  pio run -t upload
  ```

## Technical information

The first version of this VGA Testpattern generator used the [FabGL VGA16Controller](https://github.com/fdivitto/FabGL/blob/master/src/dispdrivers/vga16controller.h#L82). On higher resolutions the ESP32 didn't provide enough framebuffer memory for the full viewport, so this project tried to reduce the color depth until the framebuffer memory fully fit in to the ESP32 memory (leading to 2 colors only for `SVGA_1280x768_50Hz`).

The current version now uses [FabGL VGADirectController](https://github.com/fdivitto/FabGL/blob/master/src/dispdrivers/vgadirectcontroller.h#L77) to minimize ESP32 memory ressources for the framebuffer. It only uses memory for a small number of pre-rendered scanlines as buffer, so we can display high resolutions in full 64 colors (RGB222).

As side effects:
- we need to render each scanline on demand within the [scanline interrupt](src/TestScreen.cpp#L455-L509)
- we don't have access to the `fabgl::Canvas` drawing functions anymore, we need to render by hand
- higher CPU IRQ load due to permanent scanline rendering
- need to take care on other application activities (prevent IRQ blocking)

The minimal setup for a [VGADirectController](https://github.com/fdivitto/FabGL/blob/master/src/dispdrivers/vgadirectcontroller.h#L77) use case:
```
#include <fabgl.h>

fabgl::VGADirectController vga;

void IRAM_ATTR drawScanline(uint8_t* dest, unsigned int scanline) {
  // simply fill dest memory of current scanline with blue color
  const uint8_t col = VGAController.createRawPixel(RGB222(0,0,3));
  memset(dest, col, vga.getScreenWidth());
}

main() {
  ...
  vga.begin();
  vga.setScanlinesPerCallBack(1); // interrupt for each scanline
  vga.setDrawScanlineCallback(drawScanline);
  vga.setResolution(VGA_640x480_60Hz);
  ...
}
```

To optimize the CPU load for the test-screen rendering in the scanLine IRQ:
- pre-calculate rendering information outside the `drawScanline` interrupt (calculated on [mode change](src/TestScreen.cpp#L68-L75)):
  - use a [precalculated list](src/TestScreen.cpp#L182-L183) of X/Y coordinates for the screen grid (defining test-image areas)
  - use a [precalculated list](src/TestScreen.cpp#L161-L166) of X coordinates for each scanline for the test-screen circle
  - use [precalculated bitmaps](src/TestScreen.cpp#L141-L159) for the different test-screen portions
  - use a [list of function pointers](src/TestScreen.cpp#L174-L176) for each scanline (rendering different [portions of the test-screen](src/TestScreen.cpp#L79-L111))
- take care of the rendering order (make sure [left pixels](src/TestScreen.cpp#L467-L481) are drawn first)
- shift application activities outside screen rendering phase (vertical front porch / sync / back porch time) by synchronizing with the [vertical sync time](src/TestScreen.cpp#L511-L514)

This all is encapsulated in a [TestScreen.h](src/TestScreen.h)/[TestScreen.cpp](src/TestScreen.cpp) module. \
The minimal usage case:
```
#include <TestScreen.h>

TestScreen s(VGA_640x480_60Hz);

main() {
  ...
}
```
or with setting the resolution dynamically:

```
#include <TestScreen.h>

TestScreen s;

main() {
  ...
  s.setResolution(VGA_640x480_60Hz);
  ...
  s.setResolution(VGA_1024x768_60Hz);
}
```

## github actions

This project comes with a small github CI [integration script](.github/workflows/build.yml) to verify the PlatformIO build.

## Links
- Original [LilyGO VGA32 board](https://lilygo.cc/en-us/products/fabgl-vga32)
- Official [LilyGO FabGL](https://github.com/LilyGO/FabGL) repo (with schematics)
- More recent [fdivitto/FabGL](https://github.com/fdivitto/fabgl) repo and [project page](https://www.fabglib.org) by Fabrizio Di Vittorio
- A [collection of applications](https://github.com/fdivitto/FabGL/wiki/Made-with-FabGL) using the FabGL library
- list of [TTGO-VGA related stuff](https://github.com/iandstanley/awesome-ttgo-vga)
- FabGL [ModelineStudio](https://github.com/fdivitto/FabGL/tree/master/examples/VGA/ModelineStudio) example
- FabGL [DirectVGA](https://github.com/fdivitto/FabGL/tree/master/examples/VGA/DirectVGA) example
- my favorite [case](https://www.thingiverse.com/thing:4570375) for the VGA32 board
