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


#include <fabgl.h>

#ifndef ARRAY_SIZE
  #define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif


const int VP_BUTTON_PIN = 36; // ESP32 Pin VP

const char* PresetResolutions[] = {
    VGA_256x192_50Hz,       // Modeline for 256x192@50Hz resolution - requires upscaler
    VGA_256x384_60Hz,       // Modeline for 256x384@60Hz resolution
    VGA_320x200_60HzD,      // Modeline for 320x200@60Hz doublescan resolution
    VGA_320x200_70Hz,       // Modeline for 320x200@70Hz resolution - the same of VGA_640x200_70Hz with horizontal halved
    VGA_320x200_75Hz,       // Modeline for 320x200@75Hz resolution
    VGA_320x200_75HzRetro,  // Modeline for 320x200@75Hz retro resolution
    QVGA_320x240_60Hz,      // Modeline for 320x240@60Hz resolution
    VGA_400x300_60Hz,       // Modeline for 400x300@60Hz resolution
    VGA_480x300_75Hz,       // Modeline for 480x300@75Hz resolution
    VGA_512x192_60Hz,       // Modeline for 512x192@60Hz resolution
    VGA_512x384_60Hz,       // Modeline for 512x384@60Hz resolution
    VGA_512x448_60Hz,       // Modeline for 512x448@60Hz resolution
    VGA_512x512_58Hz,       // Modeline for 512x512@58Hz resolution
    VGA_640x200_60HzD,      // Modeline for 640x200@60Hz doublescan resolution
    VGA_640x200_70Hz,       // Modeline for 640x200@70Hz resolution - the same of VGA_640x400_70Hz with vertical halved
    VGA_640x200_70HzRetro,  // Modeline for 640x200@70Hz retro resolution
    VGA_640x240_60Hz,       // Modeline for 640x240@60Hz (DoubleScan) resolution
    NTSC_640x240_60hz,      // Modeline for 640x240@60Hz resolution - requires upscaler - Thanks to Paul Rickards
    VGA_640x350_70Hz,       // Modeline for 640x350@70Hz resolution
    VGA_640x350_70HzAlt1,   // Modeline for 640x350@70HzAlt1 resolution
    VESA_640x350_85Hz,      // Modeline for 640x350@85Hz resolution
    VGA_640x382_60Hz,       // Modeline for 640x382@60Hz resolution
    VGA_640x384_60Hz,       // Modeline for 640x384@60Hz resolution
    VGA_640x400_70Hz,       // Modeline for 640x400@70Hz resolution
    VGA_640x400_60Hz,       // Modeline for 640x400@60Hz (actually 640x480 but with less lines)
    VGA_640x480_60Hz,       // Modeline for 640x480@60Hz resolution
    VGA_640x480_60HzAlt1,   // Modeline for 640x480@60HzAlt1 resolution
    VGA_640x480_60HzD,      // Modeline for 640x480@60Hz doublescan resolution
    VGA_640x480_73Hz,       // Modeline for 640x480@73Hz resolution
    VESA_640x480_75Hz,      // Modeline for 640x480@75Hz resolution
    NTSC_720x240_60hz,      // Modeline for NTSC 720x240@60Hz resolution - requires upscaler - thanks to Paul Rickards
    PAL_720x288_50hz,       // Modeline for PAL 720x288@50Hz resolution - requires upscaler - thanks to Paul Rickards
    VGA_720x348_50HzD,      // Modeline for 720x348@50Hz doublescan resolution
    VGA_720x348_59HzD,      // Modeline for 720x348@59Hz doublescan resolution
    VGA_720x348_73Hz,       // Modeline for 720x348@73Hz resolution
    VGA_720x350_70Hz,       // Modeline for 720x350@70Hz resolution - thanks Stan Pechal
    VGA_720x400_70Hz,       // Modeline for 720x400@70Hz resolution
    VESA_720x400_85Hz,      // Modeline for 720x400@85Hz resolution
    PAL_720x576_50Hz,       // Modeline for 720x576@50Hz resolution
    PAL_768x288_50hz,       // Modeline for 768x288@50Hz resolution - requires upscaler - thanks to Paul Rickards
    VESA_768x576_60Hz,      // Modeline for 768x576@60Hz resolution
    SVGA_800x300_60Hz,      // Modeline for 800x300@60Hz resolution
    SVGA_800x600_56Hz,      // Modeline for 800x600@56Hz resolution
    SVGA_800x600_60Hz,      // Modeline for 800x600@60Hz resolution
    SVGA_960x540_60Hz,      // Modeline for 960x540@60Hz resolution
    SVGA_1024x768_60Hz,     // Modeline for 1024x768@60Hz resolution
    SVGA_1024x768_70Hz,     // Modeline for 1024x768@70Hz resolution
    SVGA_1024x768_75Hz,     // Modeline for 1024x768@75Hz resolution
    SVGA_1280x600_60Hz,     // Modeline for 1280x600@60Hz resolution
    SVGA_1280x720_60Hz,     // Modeline for 1280x720@60Hz resolution
    SVGA_1280x720_60HzAlt1, // Modeline for 1280x720@60HzAlt1 resolution
    SVGA_1280x768_50Hz      // Modeline for 1280x768@50Hz resolution
};


unsigned int currentResolution = 25;  // VGA_640x480_60Hz
int moveX = 0;
int moveY = 0;
int shrinkX = 0;
int shrinkY = 0;


fabgl::VGAPalettedController* VGAController = NULL;


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


void printInfo()
{
  auto t = VGAController->getResolutionTimings();
  Serial.printf("Modeline \"%s\" %2.8g %d %d %d %d %d %d %d %d %cHSync %cVSync %s %s\n",
                t->label,
                t->frequency / 1000000.0,
                t->HVisibleArea,
                t->HVisibleArea + t->HFrontPorch,
                t->HVisibleArea + t->HFrontPorch + t->HSyncPulse,
                t->HVisibleArea + t->HFrontPorch + t->HSyncPulse + t->HBackPorch,
                t->VVisibleArea,
                t->VVisibleArea + t->VFrontPorch,
                t->VVisibleArea + t->VFrontPorch + t->VSyncPulse,
                t->VVisibleArea + t->VFrontPorch + t->VSyncPulse + t->VBackPorch,
                t->HSyncLogic, t->VSyncLogic,
                t->scanCount == 2 ? "DoubleScan" : "",
                t->HStartingBlock == VGAScanStart::FrontPorch ? "FrontPorchBegins" :
                (t->HStartingBlock == VGAScanStart::Sync ? "SyncBegins" :
                (t->HStartingBlock == VGAScanStart::BackPorch ? "BackPorchBegins" : "VisibleBegins")));

  //Serial.printf("VFront = %d, VSync = %d, VBack = %d\n", t->VFrontPorch, t->VSyncPulse, t->VBackPorch);

  if (moveX || moveY) {
    Serial.printf("moveScreen(%d, %d)\n", moveX, moveY);
  }
  if (shrinkX || shrinkY) {
    Serial.printf("shrinkScreen(%d, %d)\n", shrinkX, shrinkY);
  }
  Serial.printf("Screen size   : %d x %d\n", VGAController->getScreenWidth(),   VGAController->getScreenHeight());
  Serial.printf("Viewport size : %d x %d\n", VGAController->getViewPortWidth(), VGAController->getViewPortHeight());
  Serial.printf("Free memory (total, min, largest): %d, %d, %d\n\n", heap_caps_get_free_size(MALLOC_CAP_32BIT),
                                                                     heap_caps_get_minimum_free_size(MALLOC_CAP_32BIT),
                                                                     heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
}

static void updateScreen()
{
  fabgl::Canvas canvas(VGAController);

  // Update the screen with an retro-style TV test image
  static const Color barColors[] = { Color::BrightWhite,   Color::BrightYellow, Color::BrightCyan, Color::BrightGreen,
                                     Color::BrightMagenta, Color::BrightRed,    Color::BrightBlue, Color::Black };
  static const Color barGrey[] = { Color::Black, Color::BrightBlack, Color::White, Color::BrightWhite };
  static const int Nx = ARRAY_SIZE(barColors); // number of color bars (=columns in test image)
  static const int Ny = 9; // number of rows in test image
  static const int F = 2;  // additional outer frame rows/columns for the grid
  const int W = canvas.getWidth();
  const int H = canvas.getHeight();
  // coordinates for the grid
  const int X[F+Nx+F+1]  = { 0, 1*W/(F+Nx+F), 2*W/(F+Nx+F), 3*W/(F+Nx+F), 4*W/(F+Nx+F), 5*W/(F+Nx+F), 6*W/(F+Nx+F), 7*W/(F+Nx+F), 8*W/(F+Nx+F), 9*W/(F+Nx+F), 10*W/(F+Nx+F), 11*W/(F+Nx+F), 12*W/(F+Nx+F)-1 };
  const int Y[F+Ny+F+1] = { 0, 1*H/(F+Ny+F), 2*H/(F+Ny+F), 3*H/(F+Ny+F), 4*H/(F+Ny+F), 5*H/(F+Ny+F), 6*H/(F+Ny+F), 7*H/(F+Ny+F), 8*H/(F+Ny+F), 9*H/(F+Ny+F), 10*H/(F+Ny+F), 11*H/(F+Ny+F), 12*H/(F+Ny+F), 13*H/(F+Ny+F)-1 };
  int i, l, x, y;

  canvas.beginUpdate();

  canvas.setBrushColor(Color::Black);
  canvas.clear();

  // draw white grid frame
  canvas.setPenColor(Color::BrightWhite);
  for (i = 0; i < ARRAY_SIZE(X); i++) { x = X[i]; canvas.drawLine(x, 0,  x,  H-1); }
  for (i = 0; i < ARRAY_SIZE(Y); i++) { y = Y[i]; canvas.drawLine(0, y, W-1,  y); }

  l=F; // draw upper color bars
  for (i = 0; i < ARRAY_SIZE(barColors); i++) {
    canvas.setBrushColor(barColors[i]);
    canvas.fillRectangle(max(X[F+i], X[F]+1), Y[l]+1, X[F+i+1] - 1, Y[l+3] - 1);
  }
  l+=3; // draw upper greyscale bars
  for (i = 0; i < ARRAY_SIZE(barGrey); i++) {
    canvas.setBrushColor(barGrey[i]);
    canvas.fillRectangle(max(X[F+2*i], X[F]+1), Y[l], X[F+2*i+2] - 1, Y[l+2] - 1);
  }

  l+=2; // draw mid white/black/white bar
  canvas.setBrushColor(Color::BrightWhite);
  canvas.fillRectangle(X[F], Y[l], X[F+Nx]-1, Y[l+1]-1);
  canvas.setBrushColor(Color::Black);
  canvas.fillRectangle(X[F+1], Y[l]+1, X[F+Nx-1]-1, Y[l+1]);
  // double vertical line in rows 3-5
  canvas.setBrushColor(Color::BrightWhite);	
  canvas.fillRectangle(X[F+Nx/2]-1, Y[l-2], X[F+Nx/2]+1, Y[l+1]);

  l++; // draw line pattern (with decrementing width/spacing)
  canvas.setBrushColor(Color::Black);
  canvas.fillRectangle(X[F]+1, Y[l], X[F+Nx]-1, Y[l+1]-1);
  canvas.setBrushColor(Color::BrightWhite);
  for (x = X[F+1]; x < X[F+Nx-1]; x += 2*i) {
    i = (X[F+Nx]-x) / (W/(F+Nx+F));
    canvas.fillRectangle(x, Y[l], x+i-1, Y[l+1]-1);
  }

  l++; // draw white bar with centered arrow
  canvas.setBrushColor(Color::BrightWhite);
  canvas.fillRectangle(X[F]+1, Y[l], X[F+Nx]-1, Y[l+1]);
  canvas.setBrushColor(Color::Black);
  const fabgl::Point arrow[] = { {W/2, Y[l+1] - 1}, {W/2 - 6, Y[l]-1}, {W/2 + 6, Y[l]-1} };
  canvas.fillPath(arrow, ARRAY_SIZE(arrow));

  l++; // text field with full 16color bar and resolution information
  static const Color cols[] = {
    Color::Black,  Color::BrightBlack,  Color::Blue,    Color::BrightBlue,
    Color::Red,    Color::BrightRed,    Color::Magenta, Color::BrightMagenta,
    Color::Green,  Color::BrightGreen,  Color::Cyan,    Color::BrightCyan,
    Color::Yellow, Color::BrightYellow, Color::White,   Color::BrightWhite
  };
  for (i = 0; i < ARRAY_SIZE(cols); i++) {
    int x1 = X[F] + (X[F+Nx]-X[F])*i/ARRAY_SIZE(cols);
    int x2 = X[F] + (X[F+Nx]-X[F])*(i+1)/ARRAY_SIZE(cols);
    canvas.setBrushColor(cols[i]);
    canvas.fillRectangle(max(x1, X[F]+1), Y[l], x2-1, Y[l+1]-1);
  }
  fabgl::VGATimings const* tim = VGAController->getResolutionTimings();
  char str[64];
  snprintf(str, sizeof(str), "\"%s\" (%.3fMHz)", tim->label, tim->frequency/1000000.0);
  size_t n = strlen(str);
  canvas.setPenColor(Color::Black);
  canvas.selectFont(W<=320 ? &fabgl::FONT_5x8 : &fabgl::FONT_8x8);
  canvas.drawText((W-n*canvas.getFontInfo()->width)/2, Y[l] + ((Y[l+1]-Y[l])-canvas.getFontInfo()->height)/2, str);

  // draw big circle
  canvas.setPenColor(Color::BrightWhite);
  canvas.drawEllipse(W/2, H/2, ((Nx+1)*W)/(F+Nx+F), ((Nx+1)*W)/(F+Nx+F));

  canvas.endUpdate();
}

static void openVGAController()
{
  // check VGA Controller with highest available color depth depending on available memory.
  // May reduce if the desired resolution can't be achieved with the current color depth
  for (int colors = 16; colors >= 2; colors /= 2)
  {
    if (VGAController) {
     VGAController->end();
     delete VGAController;
   }
   switch (colors) {
      case 16:
        VGAController = new fabgl::VGA16Controller();
        break;
      case 8:
        VGAController = new fabgl::VGA8Controller();
        break;
      case 4:
        VGAController = new fabgl::VGA4Controller();
        break;
      default:
        VGAController = new fabgl::VGA2Controller();
        break;
    }
    VGAController->begin();
    VGAController->setResolution(PresetResolutions[currentResolution]);
    if ((VGAController->getScreenHeight() - VGAController->getViewPortHeight()) <= 3) {
      break;
    }
  };
}

static void updateResolution(int inc=0)
{
  moveX = moveY = shrinkX = shrinkY = 0;
  currentResolution += inc;
  if (currentResolution >= ARRAY_SIZE(PresetResolutions)) {
    currentResolution = 0;
  }
  if (currentResolution < 0) {
    currentResolution = ARRAY_SIZE(PresetResolutions) - 1;
  }
  openVGAController();
  updateScreen();
  printInfo();
}


void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(10);

  // Button input pin
  pinMode(VP_BUTTON_PIN, INPUT_PULLUP);

  // avoid garbage into the UART
  delay(500);

  printHelp();
  updateResolution(0);
}


void loop()
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
        VGAController->moveScreen(0, -1);
        printInfo();
        break;
      case 'z':
        ++moveY;
        VGAController->moveScreen(0, +1);
        printInfo();
        break;
      case 'a':
        --moveX;
        VGAController->moveScreen(-1, 0);
        printInfo();
        break;
      case 's':
        ++moveX;
        VGAController->moveScreen(+1, 0);
        printInfo();
        break;
      case 'x':
        moveX = moveY = shrinkX = shrinkY = 0;
        VGAController->setResolution(PresetResolutions[currentResolution]);
        updateScreen();
        printInfo();
        break;
      case '+':
      case '-':
        updateResolution(c == '+' ? 1 : -1);
        break;
      case '"':
        moveX = moveY = shrinkX = shrinkY = 0;
        VGAController->setResolution( (String("\"") + Serial.readStringUntil('\n')).c_str() );
        updateScreen();
        printInfo();
        break;
      ///*
      case 'e':
      case 'E':
        t = *VGAController->getResolutionTimings();
        t.HFrontPorch = (c == 'e' ? t.HFrontPorch - 1 : t.HFrontPorch + 1);
        t.HBackPorch = (c == 'e' ? t.HBackPorch + 1 : t.HBackPorch - 1);
        VGAController->setResolution(t);
        updateScreen();
        printInfo();
        break;
      case 'r':
      case 'R':
        t = *VGAController->getResolutionTimings();
        t.HSyncPulse = (c == 'r' ? t.HSyncPulse - 1 : t.HSyncPulse + 1);
        t.HBackPorch = (c == 'r' ? t.HBackPorch + 1 : t.HBackPorch - 1);
        VGAController->setResolution(t);
        updateScreen();
        printInfo();
        break;
      case 't':
      case 'T':
        t = *VGAController->getResolutionTimings();
        t.HBackPorch = (c == 't' ? t.HBackPorch - 1 : t.HBackPorch + 1);
        t.HFrontPorch = (c == 't' ? t.HFrontPorch + 1 : t.HFrontPorch - 1);
        VGAController->setResolution(t);
        updateScreen();
        printInfo();
        break;
      case 'y':
      case 'Y':
        t = *VGAController->getResolutionTimings();
        t.VFrontPorch = (c == 'y' ? t.VFrontPorch - 1 : t.VFrontPorch + 1);
        t.VBackPorch = (c == 'y' ? t.VBackPorch + 1 : t.VBackPorch - 1);
        VGAController->setResolution(t);
        updateScreen();
        printInfo();
        break;
      case 'u':
      case 'U':
        t = *VGAController->getResolutionTimings();
        t.VSyncPulse = (c == 'u' ? t.VSyncPulse - 1 : t.VSyncPulse + 1);
        t.VBackPorch = (c == 'u' ? t.VBackPorch + 1 : t.VBackPorch - 1);
        VGAController->setResolution(t);
        updateScreen();
        printInfo();
        break;
      case 'i':
      case 'I':
        t = *VGAController->getResolutionTimings();
        t.VBackPorch = (c == 'i' ? t.VBackPorch - 1 : t.VBackPorch + 1);
        t.VFrontPorch = (c == 'i' ? t.VFrontPorch + 1 : t.VFrontPorch - 1);
        VGAController->setResolution(t);
        updateScreen();
        printInfo();
        break;
      //*/
      case 'o':
      case 'O':
        t = *VGAController->getResolutionTimings();
        t.frequency = (c == 'o' ? t.frequency - 5000 : t.frequency + 5000);
        VGAController->setResolution(t);
        updateScreen();
        printInfo();
        break;
      case '.':
        t = *VGAController->getResolutionTimings();
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
        VGAController->setResolution(t);
        updateScreen();
        printInfo();
        break;
      case 'n':
        ++shrinkX;
        VGAController->shrinkScreen(+1, 0);
        updateScreen();
        printInfo();
        break;
      case 'N':
        --shrinkX;
        VGAController->shrinkScreen(-1, 0);
        updateScreen();
        printInfo();
        break;
      case 'm':
        ++shrinkY;
        VGAController->shrinkScreen(0, +1);
        updateScreen();
        printInfo();
        break;
      case 'M':
        --shrinkY;
        VGAController->shrinkScreen(0, -1);
        updateScreen();
        printInfo();
        break;
    }
  }

  // next resulition on VP button press
  static int lastButtonState = HIGH;
  int currentButtonState = digitalRead(VP_BUTTON_PIN);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    updateResolution(1);
  }
  lastButtonState = currentButtonState;
}
