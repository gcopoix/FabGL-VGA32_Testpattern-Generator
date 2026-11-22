/*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - <http://www.fabgl.com>
  Copyright (c) 2019-2022 Fabrizio Di Vittorio.
  All rights reserved.


* Please contact fdivitto2013@gmail.com if you need a commercial license.


* This library and related software is available under GPL v3.

  FabGL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabGL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "TestScreen.h"

#ifndef ARRAY_SIZE
  #define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif


IRAM_ATTR TestScreen testScreen;

constexpr unsigned int VP_BUTTON_PIN = 36; // ESP32 Pin VP

// List of available FabGL VGAController resolutions
constexpr char const* PresetResolutions[] = {
    VGA_256x192_50Hz,       //  0: Modeline for 256x192@50Hz resolution - requires upscaler
    VGA_256x384_60Hz,       //  1: Modeline for 256x384@60Hz resolution
    VGA_320x200_60HzD,      //  2: Modeline for 320x200@60Hz doublescan resolution
    VGA_320x200_70Hz,       //  3: Modeline for 320x200@70Hz resolution - the same of VGA_640x200_70Hz with horizontal halved
    VGA_320x200_75Hz,       //  4: Modeline for 320x200@75Hz resolution
    VGA_320x200_75HzRetro,  //  5: Modeline for 320x200@75Hz retro resolution
    QVGA_320x240_60Hz,      //  6: Modeline for 320x240@60Hz resolution
    VGA_400x300_60Hz,       //  7: Modeline for 400x300@60Hz resolution
    VGA_480x300_75Hz,       //  8: Modeline for 480x300@75Hz resolution
    VGA_512x192_60Hz,       //  9: Modeline for 512x192@60Hz resolution
    VGA_512x384_60Hz,       // 10: Modeline for 512x384@60Hz resolution
    VGA_512x448_60Hz,       // 11: Modeline for 512x448@60Hz resolution
    VGA_512x512_58Hz,       // 12: Modeline for 512x512@58Hz resolution
    VGA_640x200_60HzD,      // 13: Modeline for 640x200@60Hz doublescan resolution
    VGA_640x200_70Hz,       // 14: Modeline for 640x200@70Hz resolution - the same of VGA_640x400_70Hz with vertical halved
    VGA_640x200_70HzRetro,  // 15: Modeline for 640x200@70Hz retro resolution
    VGA_640x240_60Hz,       // 16: Modeline for 640x240@60Hz (DoubleScan) resolution
    NTSC_640x240_60hz,      // 17: Modeline for 640x240@60Hz resolution - requires upscaler - Thanks to Paul Rickards
    VGA_640x350_70Hz,       // 18: Modeline for 640x350@70Hz resolution
    VGA_640x350_70HzAlt1,   // 19: Modeline for 640x350@70HzAlt1 resolution
    VESA_640x350_85Hz,      // 20: Modeline for 640x350@85Hz resolution
    VGA_640x382_60Hz,       // 21: Modeline for 640x382@60Hz resolution
    VGA_640x384_60Hz,       // 22: Modeline for 640x384@60Hz resolution
    VGA_640x400_70Hz,       // 23: Modeline for 640x400@70Hz resolution
    VGA_640x400_60Hz,       // 24: Modeline for 640x400@60Hz (actually 640x480 but with less lines)
    VGA_640x480_60Hz,       // 25: Modeline for 640x480@60Hz resolution
    VGA_640x480_60HzAlt1,   // 26: Modeline for 640x480@60HzAlt1 resolution
    VGA_640x480_60HzD,      // 27: Modeline for 640x480@60Hz doublescan resolution
    VGA_640x480_73Hz,       // 28: Modeline for 640x480@73Hz resolution
    VESA_640x480_75Hz,      // 29: Modeline for 640x480@75Hz resolution
    NTSC_720x240_60hz,      // 30: Modeline for NTSC 720x240@60Hz resolution - requires upscaler - thanks to Paul Rickards
    PAL_720x288_50hz,       // 31: Modeline for PAL 720x288@50Hz resolution - requires upscaler - thanks to Paul Rickards
    VGA_720x348_50HzD,      // 32: Modeline for 720x348@50Hz doublescan resolution
    VGA_720x348_59HzD,      // 33: Modeline for 720x348@59Hz doublescan resolution
    VGA_720x348_73Hz,       // 34: Modeline for 720x348@73Hz resolution
    VGA_720x350_70Hz,       // 35: Modeline for 720x350@70Hz resolution - thanks Stan Pechal
    VGA_720x400_70Hz,       // 36: Modeline for 720x400@70Hz resolution
    VESA_720x400_85Hz,      // 37: Modeline for 720x400@85Hz resolution
    PAL_720x576_50Hz,       // 38: Modeline for 720x576@50Hz resolution
    PAL_768x288_50hz,       // 39: Modeline for 768x288@50Hz resolution - requires upscaler - thanks to Paul Rickards
    VESA_768x576_60Hz,      // 40: Modeline for 768x576@60Hz resolution
    SVGA_800x300_60Hz,      // 41: Modeline for 800x300@60Hz resolution
    SVGA_800x600_56Hz,      // 42: Modeline for 800x600@56Hz resolution
    SVGA_800x600_60Hz,      // 43: Modeline for 800x600@60Hz resolution
    SVGA_960x540_60Hz,      // 44: Modeline for 960x540@60Hz resolution
    SVGA_1024x768_60Hz,     // 45: Modeline for 1024x768@60Hz resolution
    SVGA_1024x768_70Hz,     // 46: Modeline for 1024x768@70Hz resolution
    SVGA_1024x768_75Hz,     // 47: Modeline for 1024x768@75Hz resolution
    SVGA_1280x600_60Hz,     // 48: Modeline for 1280x600@60Hz resolution
    SVGA_1280x720_60Hz,     // 49: Modeline for 1280x720@60Hz resolution
    SVGA_1280x720_60HzAlt1, // 50: Modeline for 1280x720@60HzAlt1 resolution
    SVGA_1280x768_50Hz,     // 51: Modeline for 1280x768@50Hz resolution
#if 0
    // some additional resolutions from https://martin.hinner.info/vga/timing.html
    // as the ESP32 can emit max. 80MHz I2S rate (=pixel clock), these resolutions won't work here
    "\"1280x800@60Hz\"    83.46 1280 1344 1480 1680  800  801  804  828 -HSync +VSync", // 52: GTF
    "\"1280x960@60Hz\"   108.0  1280 1376 1488 1800  960  961  964 1000 +HSync +VSync", // 53: VESA
    "\"1280x960@60Hz\"   102.1  1280 1376 1488 1800  960  961  964 1000 +HSync +VSync", // 54: SVGA
    "\"1280x960@72Hz\"   124.54 1280 1368 1504 1728  960  961  964 1001 -HSync +VSync", // 55: GTF
    "\"1280x960@75Hz\"   129.86 1280 1368 1504 1728  960  961  964 1002 -HSync +VSync", // 56: GTF
    "\"1280x960@100Hz\"  178.99 1280 1376 1520 1760  960  961  964 1017 -HSync +VSync", // 57: GTF
    "\"1280x1024@60Hz\"  108.00 1280 1328 1440 1688 1024 1025 1028 1066 +HSync +VSync", // 58: VESA
    "\"1280x1024@75Hz\"  135.0  1280 1296 1440 1688 1024 1025 1028 1066 +HSync +VSync", // 59: VESA
    "\"1280x1024@85Hz\"  157.5  1280 1344 1504 1728 1024 1025 1028 1072 +HSync +VSync", // 60: VESA
    "\"1280x1024@100Hz\" 190.96 1280 1376 1520 1760 1024 1025 1028 1085 -HSync +VSync", // 61: GTF
    "\"1368x768@60Hz\"    85.86 1368 1440 1584 1800  768  769  772  795 -HSync +VSync", // 62: VESA

    "\"1440x900@60Hz\"   106.47 1440 1520 1672 1904  900  901  904  932 -HSync +VSync", // 63: GTF // last working?
    "\"1400x1050@60Hz\"  122.61 1400 1488 1640 1880 1050 1051 1054 1087 -HSync +VSync", // 64: GTF
    "\"1400x1050@72Hz\"  149.34 1400 1496 1648 1896 1050 1051 1054 1094 -HSync +VSync", // 65: GTF
    "\"1400x1050@75Hz\"  155.85 1400 1496 1648 1896 1050 1051 1054 1096 -HSync +VSync", // 66: GTF
    "\"1400x1050@85Hz\"  179.26 1400 1504 1656 1912 1050 1051 1054 1103 -HSync +VSync", // 67: GTF
    "\"1400x1050@100Hz\" 214.39 1400 1512 1664 1928 1050 1051 1054 1112 -HSync +VSync", // 68: GTF
    "\"1600x1200@60Hz\"	 162.0  1600 1664 1856 2160 1200 1201 1204 1250 +HSync +VSync", // 69: VESA
    "\"1600x1200@65Hz\"	 175.5  1600 1664 1856 2160 1200 1201 1204 1250 +HSync +VSync", // 70: VESA 
    "\"1600x1200@70Hz\"	 189.0  1600 1664 1856 2160 1200 1201 1204 1250 +HSync +VSync", // 71: VESA

    "\"1600x1200@75Hz\"  202.5  1600 1664 1856 2160 1200 1201 1204 1250 +HSync +VSync", // 72: VESA
    "\"1600x1200@85Hz\"  229.5  1600 1664 1856 2160 1200 1201 1204 1250 +HSync +VSync", // 73: VESA
    "\"1600x1200@100Hz\" 280.64 1600 1728 1904 2208 1200 1201 1204 1271 -HSync +VSync", // 74: GTF
    "\"1680x1050@60Hz\"  147.14 1680 1784 1968 2256 1050 1051 1054 1087 -HSync +VSync", // 75: GTF
    "\"1792x1344@60Hz\"  204.8  1792 1920 2120 2448 1344 1345 1348 1394 -HSync +VSync", // 76: VESA
    "\"1792x1344@75Hz\"  261.0  1792 1888 2104 2456 1344 1345 1348 1417 -HSync +VSync", // 77: VESA
    "\"1856x1392@60Hz\"  218.3  1856 1952 2176 2528 1392 1393 1396 1439 -HSync +VSync", // 78: VESA
    "\"1856x1392@75Hz\"  288.0  1856 1984 2208 2560 1392 1393 1396 1500 -HSync +VSync", // 79: VESA
    "\"1920x1080@60Hz\"  148.5  1920 2008 2052 2200 1080 1084 1089 1125 +HSync +VSync", // 80: 1080p
    "\"1920x1200@60Hz\"  193.16 1920 2048 2256 2592 1200 1201 1204 1242 -HSync +VSync", // 81: GTF
    "\"1920x1440@60Hz\"  234.0  1920 2048 2256 2600 1440 1441 1444 1500 -HSync +VSync", // 82: VESA
    "\"1920x1440@75Hz\"  297.0  1920 2064 2288 2640 1440 1441 1444 1500 -HSync +VSync"  // 83: VESA
#endif
  };

int currentResolution = 25;  // VGA_640x480_60Hz


static TaskHandle_t  mainTaskHandle;

void printHelp()
{
  Serial.printf("Modeline studio\n");
  Serial.printf("Chip Revision: %d   Chip Frequency: %d MHz\n\n", ESP.getChipRevision(), ESP.getCpuFreqMHz());
  Serial.printf("\nScreen move:\n");
  Serial.printf("  w = Move Up   z = Move Down   a = Move Left   s = Move Right\n");
  Serial.printf("Screen shrink:\n");
  Serial.printf("  n = Dec Horiz N = Inc Horiz   m = Dec Vert    M = Inc Vert\n");
  Serial.printf("Resolutions:\n");
  Serial.printf("  + = Next resolution  - = Prev resolution   x = Restore modeline\n");
  Serial.printf("Modeline change:\n");
  Serial.printf("  Just insert a modline. Example: \"640x350@70Hz\" 25.175 640 656 752 800 350 366 462 510 -HSync -VSync\n");
  ///*
  Serial.printf("  e = Decrease horiz. Front Porch   E = Increase horiz. Front Porch\n");
  Serial.printf("  r = Decrease horiz. Sync Pulse    R = Increase horiz. Sync Pulse\n");
  Serial.printf("  t = Decrease horiz. Bach Porch    T = Increase horiz. Back Porch\n");
  Serial.printf("  y = Decrease vert. Front Porch    Y = Increase vert. Front Porch\n");
  Serial.printf("  u = Decrease vert. Sync Pulse     U = Increase vert. Sync Pulse\n");
  Serial.printf("  i = Decrease vert. Bach Porch     I = Increase vert. Back Porch\n");
  //*/
  Serial.printf("  o = Decrease frequency by 5KHz    O = Increase frequency by 5KHz\n");
  Serial.printf("  . = Change horiz. signals order\n");
  Serial.printf("Various:\n");
  Serial.printf("  h = Print This help\n\n");
}

int moveX = 0;
int moveY = 0;
int shrinkX = 0;
int shrinkY = 0;

void printInfo()
{
  auto t = testScreen.getResolutionTimings();
  Serial.printf("Modeline \"%s\" %2.8g %d %d %d %d %d %d %d %d %cHSync %cVSync %s %s\n",
                t.label,
                t.frequency / 1000000.0,
                t.HVisibleArea,
                t.HVisibleArea + t.HFrontPorch,
                t.HVisibleArea + t.HFrontPorch + t.HSyncPulse,
                t.HVisibleArea + t.HFrontPorch + t.HSyncPulse + t.HBackPorch,
                t.VVisibleArea,
                t.VVisibleArea + t.VFrontPorch,
                t.VVisibleArea + t.VFrontPorch + t.VSyncPulse,
                t.VVisibleArea + t.VFrontPorch + t.VSyncPulse + t.VBackPorch,
                t.HSyncLogic, t.VSyncLogic,
                t.scanCount == 2 ? "DoubleScan" : "",
                t.HStartingBlock == VGAScanStart::FrontPorch ? "FrontPorchBegins" :
               (t.HStartingBlock == VGAScanStart::Sync ? "SyncBegins" :
               (t.HStartingBlock == VGAScanStart::BackPorch ? "BackPorchBegins" : "VisibleBegins")));

  //Serial.printf("VFront = %d, VSync = %d, VBack = %d\n", t->VFrontPorch, t->VSyncPulse, t->VBackPorch);

  if (moveX || moveY) {
    Serial.printf("moveScreen(%d, %d)\n", moveX, moveY);
  }
  if (shrinkX || shrinkY) {
    Serial.printf("shrinkScreen(%d, %d)\n", shrinkX, shrinkY);
  }
  Serial.printf("Screen size  : %d x %d\n", testScreen.getScreenWidth(),   testScreen.getScreenHeight());
  Serial.printf("Free memory (total, min, largest): %d, %d, %d\n\n", heap_caps_get_free_size(MALLOC_CAP_32BIT),
                                                                     heap_caps_get_minimum_free_size(MALLOC_CAP_32BIT),
                                                                     heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
}

void updateResolution(int inc=0)
{
  moveX = moveY = shrinkX = shrinkY = 0;
  currentResolution += inc;
  if (currentResolution >= ARRAY_SIZE(PresetResolutions)) {
    currentResolution = 0;
  }
  if (currentResolution < 0) {
    currentResolution = ARRAY_SIZE(PresetResolutions) - 1;
  }

  testScreen.setResolution(PresetResolutions[currentResolution]);

  printInfo();
}


void checkInputs()
{
  fabgl::VGATimings t;
  if (Serial.available() > 0) {
    char c = Serial.read();
    switch (c) {
      case 'h':
        printHelp();
        break;
      case 'w':
        --moveY;
        testScreen.moveScreen(0, -1);
        printInfo();
        break;
      case 'z':
        ++moveY;
        testScreen.moveScreen(0, +1);
        printInfo();
        break;
      case 'a':
        --moveX;
        testScreen.moveScreen(-1, 0);
        printInfo();
        break;
      case 's':
        ++moveX;
        testScreen.moveScreen(+1, 0);
        printInfo();
        break;
      case 'x':
        moveX = moveY = shrinkX = shrinkY = 0;
        updateResolution(0);
        printInfo();
        break;
      case '+':
      case '-':
        updateResolution(c == '+' ? 1 : -1);
        break;
      case '"':
        moveX = moveY = shrinkX = shrinkY = 0;
        testScreen.setResolution( (String("\"") + Serial.readStringUntil('\n')).c_str() );
        printInfo();
        break;
      ///*
      case 'e':
      case 'E':
        t = testScreen.getResolutionTimings();
        t.HFrontPorch = (c == 'e' ? t.HFrontPorch - 1 : t.HFrontPorch + 1);
        t.HBackPorch = (c == 'e' ? t.HBackPorch + 1 : t.HBackPorch - 1);
        testScreen.setResolution(t);
        printInfo();
        break;
      case 'r':
      case 'R':
        t = testScreen.getResolutionTimings();
        t.HSyncPulse = (c == 'r' ? t.HSyncPulse - 1 : t.HSyncPulse + 1);
        t.HBackPorch = (c == 'r' ? t.HBackPorch + 1 : t.HBackPorch - 1);
        testScreen.setResolution(t);
        printInfo();
        break;
      case 't':
      case 'T':
        t = testScreen.getResolutionTimings();
        t.HBackPorch = (c == 't' ? t.HBackPorch - 1 : t.HBackPorch + 1);
        t.HFrontPorch = (c == 't' ? t.HFrontPorch + 1 : t.HFrontPorch - 1);
        testScreen.setResolution(t);
        printInfo();
        break;
      case 'y':
      case 'Y':
        t = testScreen.getResolutionTimings();
        t.VFrontPorch = (c == 'y' ? t.VFrontPorch - 1 : t.VFrontPorch + 1);
        t.VBackPorch = (c == 'y' ? t.VBackPorch + 1 : t.VBackPorch - 1);
        testScreen.setResolution(t);
        printInfo();
        break;
      case 'u':
      case 'U':
        t = testScreen.getResolutionTimings();
        t.VSyncPulse = (c == 'u' ? t.VSyncPulse - 1 : t.VSyncPulse + 1);
        t.VBackPorch = (c == 'u' ? t.VBackPorch + 1 : t.VBackPorch - 1);
        testScreen.setResolution(t);
        printInfo();
        break;
      case 'i':
      case 'I':
        t = testScreen.getResolutionTimings();
        t.VBackPorch = (c == 'i' ? t.VBackPorch - 1 : t.VBackPorch + 1);
        t.VFrontPorch = (c == 'i' ? t.VFrontPorch + 1 : t.VFrontPorch - 1);
        testScreen.setResolution(t);
        printInfo();
        break;
      //*/
      case 'o':
      case 'O':
        t = testScreen.getResolutionTimings();
        t.frequency = (c == 'o' ? t.frequency - 5000 : t.frequency + 5000);
        testScreen.setResolution(t);
        printInfo();
        break;
      case '.':
        t = testScreen.getResolutionTimings();
        switch (t.HStartingBlock) {
          case VGAScanStart::FrontPorch:
            t.HStartingBlock = VGAScanStart::Sync;
            break;
          case VGAScanStart::Sync:
            t.HStartingBlock = VGAScanStart::BackPorch;
            break;
          case VGAScanStart::BackPorch:
            t.HStartingBlock = VGAScanStart::VisibleArea;
            break;
          case VGAScanStart::VisibleArea:
            t.HStartingBlock = VGAScanStart::FrontPorch;
            break;
        }
        testScreen.setResolution(t);
        printInfo();
        break;
      case 'n':
        ++shrinkX;
        testScreen.shrinkScreen(+1, 0);
        printInfo();
        break;
      case 'N':
        --shrinkX;
        testScreen.shrinkScreen(-1, 0);
        printInfo();
        break;
      case 'm':
        ++shrinkY;
        testScreen.shrinkScreen(0, +1);
        printInfo();
        break;
      case 'M':
        --shrinkY;
        testScreen.shrinkScreen(0, -1);
        printInfo();
        break;
    }
  }
}

void checkButton()
{
  static int lastButtonState = HIGH;
  // next resolution on VP button press
  int currentButtonState = digitalRead(VP_BUTTON_PIN);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    updateResolution(1);
  }
  lastButtonState = currentButtonState;
}

void loop()
{
  checkInputs();

  checkButton();

  // wait for vertical sync (reserve displaytime for drawing)
  xSemaphoreTake(testScreen.getVerticalSyncSemaphore(), 0xFFFF);
}

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(10);

  mainTaskHandle = xTaskGetCurrentTaskHandle();

  // avoid garbage into the UART
  delay(50);

  // Button input pin
  pinMode(VP_BUTTON_PIN, INPUT_PULLUP);

  // set initial resolution
  updateResolution();

  printHelp();
}
