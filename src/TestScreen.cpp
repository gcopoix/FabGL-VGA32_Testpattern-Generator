#include "TestScreen.h"

#ifndef ARRAY_SIZE
  #define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

class TestScreen::Impl
{
  public:
    Impl()
        : m_scanlinesPerCallback(2)
        , m_font(nullptr)
        , m_bitmap_grid_line(nullptr)
        , m_bitmap_colorbar_line(nullptr)
        , m_bitmap_greybar_line(nullptr)
        , m_bitmap_stripes_line(nullptr)
        , m_bitmap_fullcolor_line(nullptr)
        , m_bitmap_ScreenInfo(nullptr)
        , m_cbScanLine(nullptr)
        , m_circleCoordinates(nullptr)
        , m_mainColors({RGB222(RGB888(Color::Black)), RGB222(RGB888(Color::BrightWhite))})
        , m_gridStartScanLineIdx(0)
        , m_verticalSyncSemaphore(xSemaphoreCreateBinary())
    {
        vga.begin();
        vga.setScanlinesPerCallBack(m_scanlinesPerCallback);
        vga.setDrawScanlineCallback(drawScanlineIRQ, this);
    };

    ~Impl() {
        vga.end();
        m_cbScanLine = nullptr;
    }

    void setResolution(fabgl::VGATimings const& timings)
    {
        unsigned int i, y;

        // invalidate scanline callback list which disables scanLine IRQ handling
        m_cbScanLine = nullptr;

        // invalidate buffers
        m_bitmap_grid_line = nullptr;
        m_bitmap_colorbar_line = nullptr;
        m_bitmap_greybar_line = nullptr;
        m_bitmap_stripes_line = nullptr;
        m_bitmap_fullcolor_line = nullptr;
        m_bitmap_ScreenInfo = nullptr;
        m_circleCoordinates = nullptr;

        // set new resolution and update variables and buffers
        vga.setResolution(timings);

        W = vga.getScreenWidth();
        H = vga.getScreenHeight();
        m_bgcolor = vga.createRawPixel(m_mainColors[0]);
        m_fgcolor = vga.createRawPixel(m_mainColors[1]);

        // update grid lines coordination arrays make columns dword aligned due to pixel format handling
        // see https://github.com/fdivitto/FabGL/blob/master/src/dispdrivers/vgabasecontroller.h#L64-L67
        for (i = 0; i < ARRAY_SIZE(X); i++) {
            X[i] = min(bitmap_align(i*W/(Fx+Nx+Fx)), W-1);
        }
        for (i = 0; i < ARRAY_SIZE(Y); i++) {
            Y[i] = min(i*H/(Fy+Ny+Fy), H-1);
        }

        // do some pre-rendering to optimize scanline drawing within scanLine interrupt
        renderGridLine();
        renderColorBarLine();
        renderGreyBarLine();
        renderStripesLine();
        renderFullColorLine();
        renderScreenInfo();
        updateCircleCoordinates();

        // prepare scanline callback list for current resolution
        std::unique_ptr<lineCB[]> cbScanLine = std::unique_ptr<lineCB[]>(new lineCB[H]);
        for (i=0; i < ARRAY_SIZE(Y)-1; i++) {
            cbScanLine[Y[i]] = &Impl::drawWhiteLine;
            if (i >= Fy && i < Fy+3) {
                for (y=Y[Fy]+1; y < Y[i+1]; y++) {
                    cbScanLine[y] = &Impl::drawColorBarLine;
                }
            } else if (i >= Fy+3 && i < Fy+5) {
                for (y=min(Y[Fy+3]+1, Y[i]); y < Y[i+1]; y++) {
                    cbScanLine[y] = &Impl::drawGreyBarLine;
                }
            } else if (i >= Fy+5 && i < Fy+6) {
                for (y=Y[i]+1; y < Y[i+1]; y++) {
                    cbScanLine[y] = &Impl::drawWhiteBlackWhiteLine;
                }
            } else if (i >= Fy+6 && i < Fy+7) {
                for (y=Y[i]+0; y < Y[i+1]; y++) {
                    cbScanLine[y] = &Impl::drawVerticalStripesLine;
                }
            } else if (i >= Fy+7 && i < Fy+8) {
                for (y=Y[i]+0; y < Y[i+1]; y++) {
                    cbScanLine[y] = &Impl::drawArrowLine;
                }
            } else if (i >= Fy+8 && i < Fy+9) {
                for (y=Y[i]+1; y < Y[i+1]; y++) {
                    cbScanLine[y] = &Impl::drawFullColorInfoLine;
                }
            } else {
                for (y=Y[i]+1; y < Y[i+1]; y++) {
                    cbScanLine[y] = &Impl::drawGridLine;
                }
            }
        }
        cbScanLine[Y[ARRAY_SIZE(Y)-1]] = &Impl::drawWhiteLine;

        // make scanline callback list available for current resolution.
        // this re-enables scanLine IRQ handling
        m_cbScanLine = std::move(cbScanLine);
    }

    fabgl::VGATimings const& getResolutionTimings()
    {
        return *vga.getResolutionTimings();
    }

    void setBGColor(RGB222 color) { m_bgcolor = vga.createRawPixel(m_mainColors[0] = color); }
    void setFGColor(RGB222 color) { m_fgcolor = vga.createRawPixel(m_mainColors[1] = color); }

    // binary semaphone - can be used by application to synchronize with the vertical sync
    SemaphoreHandle_t      m_verticalSyncSemaphore;

    fabgl::VGADirectController vga;

private:

    unsigned int m_scanlinesPerCallback;

    // pointer to current selected font
    fabgl::FontInfo const* m_font;


    // text in the screen info line
    char                   m_str[40];
    // bitmap buffer for the pre-rendered video mode text
    std::unique_ptr<uint8_t[]> m_bitmap_ScreenInfo;
    // line length of rendered ScreenInfo bitmap line
    size_t                 m_ScreenInfo_length;

    // bitmap buffer for the pre-rendered grid line
    std::unique_ptr<uint8_t[]> m_bitmap_grid_line;

    // bitmap buffer for the pre-rendered top color bar line
    std::unique_ptr<uint8_t[]> m_bitmap_colorbar_line;

    // bitmap buffer for the pre-rendered top grey bar line
    std::unique_ptr<uint8_t[]> m_bitmap_greybar_line;

    // bitmap buffer for the pre-rendered vertical stripes line
    std::unique_ptr<uint8_t[]> m_bitmap_stripes_line;

    // bitmap buffer for the pre-rendered full-color line
    std::unique_ptr<uint8_t[]> m_bitmap_fullcolor_line;

    // x coordinates for big test-screen
    struct CircleCoordinate {
        unsigned int x;
        unsigned int len;
    };
    std::unique_ptr<struct CircleCoordinate[]> m_circleCoordinates;

    // Y index of the start of the current grid line (updated in Scanline IRQ)
    int                    m_gridStartScanLineIdx;

    // foreground and background colors
    uint8_t                m_bgcolor, m_fgcolor;

    // drawing callback list for each scanline
    typedef void (Impl::*lineCB)(uint8_t*, unsigned int);
    std::unique_ptr<lineCB[]> m_cbScanLine;

    RGB222                 m_mainColors[2];

    unsigned int W;             // width of the screen
    unsigned int H;             // height of the screen
    unsigned int X[Fx+Nx+Fx+1]; // array of x coordinates for the (DWORD aligned) grid columns
    unsigned int Y[Fy+Ny+Fy+1]; // array of y coordinates for the (DWORD aligned) grid rows


    // beware the pixel arrangement in memory, the framebuffer line is not linear:
    // pixel 0 = byte 2, pixel 1 = byte 3, pixel 2 = byte 0, pixel 3 = byte 1 :
    // pixel : 0  1  2  3  4  5  6  7  8  9 10 11 ...
    // byte  : 2  3  0  1  6  7  4  5 10 11  8  9 ...

    // make coordnate DWORD aligned
    inline unsigned int bitmap_align(unsigned int x)
    {
        return (x + 3) & ~3;
    }

    // pre-render screen information in m_bitmap_ScreenInfo
    void renderScreenInfo()
    {
        fabgl::VGATimings const& tim = *vga.getResolutionTimings();
        snprintf(m_str, sizeof(m_str), "\"%s\" (%.3fMHz)", tim.label, tim.frequency/1000000.0);
        unsigned int n = strlen(m_str);

        m_font = (W<=320 ? &fabgl::FONT_5x8 : &fabgl::FONT_8x8);
        unsigned int stride = bitmap_align(n*m_font->width);
        m_bitmap_ScreenInfo = std::unique_ptr<uint8_t[]>(new uint8_t[10*stride]);
        memset(&m_bitmap_ScreenInfo[0], m_bgcolor, 10*stride);

        for (unsigned int i=0; i < n; i++) {
            char c = m_str[i];
            for (unsigned int y=0; y < m_font->height; y++) {
                uint8_t* dstline = &m_bitmap_ScreenInfo[(1+y)*stride];
                for (unsigned int x=0; x < m_font->width; x++) {
                    if (m_font->data[8*c + y] & (0x80 >> x)) {
                        VGA_PIXELINROW(dstline, i*m_font->width + x) = m_fgcolor;
                    }
                }
            }
        }
    }

    // pre-render complete grid line in m_bitmap_grid_line
    void renderGridLine()
    {
        m_bitmap_grid_line = std::unique_ptr<uint8_t[]>(new uint8_t[bitmap_align(W)]);

        for (unsigned int i = 0; i < ARRAY_SIZE(X)-1; i++) {
            memset(&m_bitmap_grid_line[X[i]], m_bgcolor, bitmap_align(X[i+1]-X[i]));
            VGA_PIXELINROW(m_bitmap_grid_line, X[i]) = m_fgcolor;
        }
        // and finally right-most grid column pixel
        VGA_PIXELINROW(m_bitmap_grid_line, X[ARRAY_SIZE(X)-1]) = m_fgcolor;
    }

    // pre-rendercolor bar line in m_bitmap_colorbar_line
    void renderColorBarLine()
    {
        static const DRAM_ATTR RGB222 colors[] = { RGB222(RGB888(Color::BrightWhite)),   RGB222(RGB888(Color::BrightYellow)),
                                                   RGB222(RGB888(Color::BrightCyan)),    RGB222(RGB888(Color::BrightGreen)),
                                                   RGB222(RGB888(Color::BrightMagenta)), RGB222(RGB888(Color::BrightRed)),
                                                   RGB222(RGB888(Color::BrightBlue)),    RGB222(RGB888(Color::Black)) };
        static_assert(ARRAY_SIZE(colors) == Nx, "Number of colors must match Nx");

        m_bitmap_colorbar_line = std::unique_ptr<uint8_t[]>(new uint8_t[X[Fx+Nx]-X[Fx]]);

        for (unsigned int i=0; i < Nx; i++) {
            uint8_t col = vga.createRawPixel(colors[i]);
            memset(&m_bitmap_colorbar_line[X[Fx+i]-X[Fx]], col, X[Fx+i+1]-X[Fx+i]);
            if (i == 0) {
                VGA_PIXELINROW(m_bitmap_colorbar_line, 0) = m_fgcolor;
            }
        }
    }

    // pre-render grey bar line in m_bitmap_greybar_line
    void renderGreyBarLine()
    {
        static const RGB222 grey[] = { RGB222(RGB888(Color::Black)), RGB222(RGB888(Color::BrightBlack)),
                                       RGB222(RGB888(Color::White)), RGB222(RGB888(Color::BrightWhite)) };
        static_assert(ARRAY_SIZE(grey) == Nx/2, "Number of grey colors must match Nx/2");

        m_bitmap_greybar_line = std::unique_ptr<uint8_t[]>(new uint8_t[X[Fx+Nx]-X[Fx]]);

        for (unsigned int i=0; i<Nx; i++) {
            uint8_t col = vga.createRawPixel(grey[i/2]);
            memset(&m_bitmap_greybar_line[X[Fx+i]-X[Fx]], col, X[Fx+i+1]-X[Fx+i]);
            if (i == 0) {
                VGA_PIXELINROW(m_bitmap_greybar_line, 0) = m_fgcolor;
            }
        }
        drawCenterDots(&m_bitmap_greybar_line[0], m_fgcolor, W/320);
    }

    // pre-render vertical stripes line in m_bitmap_stripes_line
    void renderStripesLine()
    {
        const uint8_t pattern[][24] = { { m_bgcolor, m_bgcolor, m_bgcolor, m_bgcolor,  m_fgcolor, m_fgcolor, m_bgcolor, m_bgcolor, // 6x black, 6x white, ...
                                          m_fgcolor, m_fgcolor, m_fgcolor, m_fgcolor,  m_bgcolor, m_bgcolor, m_bgcolor, m_bgcolor,
                                          m_fgcolor, m_fgcolor, m_bgcolor, m_bgcolor,  m_fgcolor, m_fgcolor, m_fgcolor, m_fgcolor },
                                        { m_bgcolor, m_bgcolor, m_bgcolor, m_bgcolor,  m_fgcolor, m_fgcolor, m_fgcolor, m_fgcolor, // 4x black, 4x white, ...
                                          m_bgcolor, m_bgcolor, m_bgcolor, m_bgcolor,  m_fgcolor, m_fgcolor, m_fgcolor, m_fgcolor,
                                          m_bgcolor, m_bgcolor, m_bgcolor, m_bgcolor,  m_fgcolor, m_fgcolor, m_fgcolor, m_fgcolor },
                                        { m_bgcolor, m_fgcolor, m_bgcolor, m_bgcolor,  m_bgcolor, m_bgcolor, m_fgcolor, m_fgcolor, // 3x black, 3x white, ...
                                          m_fgcolor, m_fgcolor, m_bgcolor, m_fgcolor,  m_bgcolor, m_fgcolor, m_bgcolor, m_bgcolor,
                                          m_bgcolor, m_bgcolor, m_fgcolor, m_fgcolor,  m_fgcolor, m_fgcolor, m_bgcolor, m_fgcolor },
                                        { m_fgcolor, m_fgcolor, m_bgcolor, m_bgcolor,  m_fgcolor, m_fgcolor, m_bgcolor, m_bgcolor, // 2x black, 2x white, ...
                                          m_fgcolor, m_fgcolor, m_bgcolor, m_bgcolor,  m_fgcolor, m_fgcolor, m_bgcolor, m_bgcolor,
                                          m_fgcolor, m_fgcolor, m_bgcolor, m_bgcolor,  m_fgcolor, m_fgcolor, m_bgcolor, m_bgcolor },
                                        { m_bgcolor, m_fgcolor, m_bgcolor, m_fgcolor,  m_bgcolor, m_fgcolor, m_bgcolor, m_fgcolor, // 1x black, 1x white, ...
                                          m_bgcolor, m_fgcolor, m_bgcolor, m_fgcolor,  m_bgcolor, m_fgcolor, m_bgcolor, m_fgcolor,
                                          m_bgcolor, m_fgcolor, m_bgcolor, m_fgcolor,  m_bgcolor, m_fgcolor, m_bgcolor, m_fgcolor } };

        m_bitmap_stripes_line = std::unique_ptr<uint8_t[]>(new uint8_t[X[Fx+Nx]-X[Fx]+sizeof(pattern[0])]);

        unsigned int x = 0, w = bitmap_align((X[Fx+1]-X[Fx])/2);
        memset(&m_bitmap_stripes_line[0], m_fgcolor, X[Fx+Nx]-X[Fx]);

        for (x = w; x < X[Fx+Nx]-X[Fx]-w; x+=sizeof(pattern[0])) {
            size_t idx = W<=400 ? ((ARRAY_SIZE(pattern)-1)*(x-w))/(X[Fx+Nx]-(X[Fx]+2*w))+1 // don't use 1st pattern for low resolutions
                                :  (ARRAY_SIZE(pattern)   *(x-w))/(X[Fx+Nx]-(X[Fx]+2*w));
            memcpy(&m_bitmap_stripes_line[x], pattern[idx], sizeof(pattern[0]));
        }
    }

    // pre-render full-colored line in m_bitmap_fullcolor_line
    void renderFullColorLine()
    {
        static const RGB222 full_colors[] = {
        #if 1
            RGB222(0,0,0), RGB222(0,1,0), RGB222(0,2,0), RGB222(0,3,0),
            RGB222(1,3,0), RGB222(1,2,0), RGB222(1,1,0), RGB222(1,0,0),
            RGB222(2,0,0), RGB222(2,1,0), RGB222(2,2,0), RGB222(2,3,0),
            RGB222(3,3,0), RGB222(3,2,0), RGB222(3,1,0), RGB222(3,0,0),

            RGB222(3,0,1), RGB222(3,1,1), RGB222(3,2,1), RGB222(3,3,1),
            RGB222(2,3,1), RGB222(2,2,1), RGB222(2,1,1), RGB222(2,0,1),
            RGB222(1,0,1), RGB222(1,1,1), RGB222(1,2,1), RGB222(1,3,1),
            RGB222(0,3,1), RGB222(0,2,1), RGB222(0,1,1), RGB222(0,0,1),

            RGB222(0,0,2), RGB222(0,1,2), RGB222(0,2,2), RGB222(0,3,2),
            RGB222(1,3,2), RGB222(1,2,2), RGB222(1,1,2), RGB222(1,0,2),
            RGB222(2,0,2), RGB222(2,1,2), RGB222(2,2,2), RGB222(2,3,2),
            RGB222(3,3,2), RGB222(3,2,2), RGB222(3,1,2), RGB222(3,0,2),

            RGB222(3,0,3), RGB222(3,1,3), RGB222(3,2,3), RGB222(3,3,3),
            RGB222(2,3,3), RGB222(2,2,3), RGB222(2,1,3), RGB222(2,0,3),
            RGB222(1,0,3), RGB222(1,1,3), RGB222(1,2,3), RGB222(1,3,3),
            RGB222(0,3,3), RGB222(0,2,3), RGB222(0,1,3), RGB222(0,0,3),
        #else
            RGB222(RGB888(Color::Black)),   RGB222(RGB888(Color::BrightBlack)),
            RGB222(RGB888(Color::Blue)),    RGB222(RGB888(Color::BrightBlue)),
            RGB222(RGB888(Color::Red)),     RGB222(RGB888(Color::BrightRed)),
            RGB222(RGB888(Color::Magenta)), RGB222(RGB888(Color::BrightMagenta)),
            RGB222(RGB888(Color::Green)),   RGB222(RGB888(Color::BrightGreen)),
            RGB222(RGB888(Color::Cyan)),    RGB222(RGB888(Color::BrightCyan)),
            RGB222(RGB888(Color::Yellow)),  RGB222(RGB888(Color::BrightYellow)),
            RGB222(RGB888(Color::White)),   RGB222(RGB888(Color::BrightWhite))
        #endif
        };

        m_bitmap_fullcolor_line = std::unique_ptr<uint8_t[]>(new uint8_t[X[Fx+Nx]-X[Fx]]);

        unsigned int i, x;

        // render middle full-color part
        VGA_PIXELINROW(m_bitmap_fullcolor_line, 0) = m_fgcolor;
        for (x = 1; x < X[Fx+Nx]-X[Fx]; x++) {
            unsigned int idx = ARRAY_SIZE(full_colors)*(x)/(X[Fx+Nx]-X[Fx]);
            const uint8_t color = vga.createRawPixel(full_colors[idx]);
            VGA_PIXELINROW(m_bitmap_fullcolor_line, x) = color;
        }
    }

    // generate circle coordinates for each scanline in m_circleCoordinates
    void updateCircleCoordinates()
    {
        m_circleCoordinates = std::unique_ptr<struct CircleCoordinate[]>(new struct CircleCoordinate[H]);

        int r = (Nx+1)*W/(2*(Fx+Nx+Fx));
        for (unsigned int i=0; i < H; i++) {
            int x0 = sqrt(r*r - (i-H/2)*(i-H/2));
            int x1 = sqrt(r*r - ((i+1)-H/2)*((i+1)-H/2));
            if (x0 == 65535 || x1 == 65535) {
                m_circleCoordinates[i].x = 0;
                m_circleCoordinates[i].len = 0;
            } else {
                m_circleCoordinates[i].x = min(x0, x1);
                m_circleCoordinates[i].len = max(1, abs(x1-x0));
            }
        }
    }


    // line drawing callbacks: Draw white portion of line
    void drawWhiteLine(uint8_t* dest, unsigned int scanLine)
    {
        const uint8_t fgcolor = vga.createRawPixel(m_mainColors[1]);
        memset(dest, fgcolor, X[Fx+Nx]-X[Fx]);
    }

    // line drawing callbacks: Draw grid portion of line
    void drawGridLine(uint8_t* dest, unsigned int scanLine)
    {
        memcpy(dest, &m_bitmap_grid_line[X[Fx]], X[Fx+Nx]-X[Fx]);
    }

    // line drawing callbacks: Draw color bar portion of line
    void drawColorBarLine(uint8_t* dest, unsigned int scanLine)
    {
        memcpy(dest, &m_bitmap_colorbar_line[0], X[Fx+Nx]-X[Fx]);
    }

    // drawing centered dot(s) with given color in current scanline
    inline void drawCenterDots(uint8_t* dest, uint8_t color, unsigned int width)
    {
        unsigned int x0 = W/2-X[Fx]; // center of the inner testimage bitmap part
        for (unsigned int x=x0-width; x<=x0+width; x++) {
            VGA_PIXELINROW(dest, x) = color;
        }
    }

    // line drawing callbacks: Draw grey bar portion of line
    void drawGreyBarLine(uint8_t* dest, unsigned int scanLine)
    {
        memcpy(dest, &m_bitmap_greybar_line[0], X[Fx+Nx]-X[Fx]);
    }

    // line drawing callbacks: Draw white-black-white portion of line
    void drawWhiteBlackWhiteLine(uint8_t* dest, unsigned int scanLine)
    {
        for (size_t i=0; i < Nx; i++) {
            uint8_t col = (i >= 1 && i < Nx-1) ? m_bgcolor : m_fgcolor;
            memset(dest+X[Fx+i]-X[Fx], col, X[Fx+i+1]-X[Fx+i]);
        }
        drawCenterDots(dest, m_fgcolor, W/320);
    }

    // line drawing callbacks: Draw resolution stripes pattern portion of portion of line
    void drawVerticalStripesLine(uint8_t* dest, unsigned int scanLine)
    {
        memcpy(dest, &m_bitmap_stripes_line[0], X[Fx+Nx]-X[Fx]);
    }

    // line drawing callbacks: Draw white portion of line with centered arrow
    void drawArrowLine(uint8_t* dest, unsigned int scanLine)
    {
        memset(dest, m_fgcolor, X[Fx+Nx]-X[Fx]);

        unsigned int i = m_gridStartScanLineIdx;
        unsigned int width = (W/100)*(Y[i+1]-(scanLine+1)) / (Y[i+1]-Y[i]);
        drawCenterDots(dest, m_bgcolor, width);
    }

    //  line drawing callbacks: Draw all colors portion of line with resolution information
    void drawFullColorInfoLine(uint8_t* dest, unsigned int scanLine)
    {
        unsigned int i = m_gridStartScanLineIdx;
        int y = scanLine - (Y[i] + (Y[i+1]-Y[i] - m_font->height)/2);
        if (y >= 0 && y < 1+m_font->height+1) {
            // draw centered ScreeInfo line from pre-rendered bitmap
            unsigned int stride = bitmap_align(strlen(m_str)*m_font->width);
            unsigned int x0 = bitmap_align((X[Fx+Nx]-X[Fx]-stride)/2);
            // 1st: get pre-rendered left part of full color line
            memcpy(dest, &m_bitmap_fullcolor_line[0], x0);
            // 2nd: get pre-rendered screeninfo line
            memcpy(dest + x0, &m_bitmap_ScreenInfo[y*stride], stride);
            // 3rd: get pre-rendered right part of full color line
            memcpy(dest + x0 + stride, &m_bitmap_fullcolor_line[x0 + stride], (X[Fx+Nx]-X[Fx])-(x0 + stride));
        } else {
            // get pre-rendered full color line
            memcpy(dest, &m_bitmap_fullcolor_line[0], X[Fx+Nx]-X[Fx]);
        }
    }

    void drawScanlineIRQ(uint8_t* dest, int scanLine)
    {
        if (m_cbScanLine) {
            for (unsigned int i, n, r, r0 = X[Fx+Nx/2], sl = 0; sl < m_scanlinesPerCallback; sl++, scanLine++, dest += W) {
                // evaluate grid start scanline index corresponding to the current scanLine
                for (i=0; i < ARRAY_SIZE(Y); i++) {
                    if (scanLine >= Y[i] && scanLine < Y[i+1]) {
                        m_gridStartScanLineIdx = i;
                        break;
                    }
                }

                // draw scanline in 3 steps:

                // 1. transfer common left rows of scanline from pre-rendered grid line first
                if (scanLine == Y[m_gridStartScanLineIdx]) {
                    memset(dest, m_fgcolor, X[Fx]-X[0]);
                } else {
                    memcpy(dest+X[0], &m_bitmap_grid_line[X[0]], X[Fx]-X[0]);
                }
                r = r0-X[Fx];
                if (m_circleCoordinates[scanLine].x > r) {
                    // print circle in left frame columns only
                    for (n = 0; n < m_circleCoordinates[scanLine].len; n++) {
                        VGA_PIXELINROW(dest, r0-m_circleCoordinates[scanLine].x+n) = m_fgcolor;
                    }
                }

                // 2. draw middle testimage-specific portion of scanline by line-specific method
                (this->*m_cbScanLine[scanLine])(dest+X[Fx], scanLine);
                r = X[Fx+Nx]-r0;
                if (m_circleCoordinates[scanLine].x < r) {
                    // print circle in middle test-screen columns only
                    for (n = 0; n < m_circleCoordinates[scanLine].len; n++) {
                        VGA_PIXELINROW(dest, r0 - m_circleCoordinates[scanLine].x - n) = m_fgcolor;
                        VGA_PIXELINROW(dest, r0 + m_circleCoordinates[scanLine].x + n) = m_fgcolor;
                    }
                }

                // 3. transfer common right rows of scanline from pre-rendered grid line
                if (scanLine == Y[m_gridStartScanLineIdx]) {
                    memset(dest+X[Fx+Nx], m_fgcolor, bitmap_align(X[ARRAY_SIZE(X)-1]-X[Fx+Nx]));
                } else {
                    memcpy(dest+X[Fx+Nx], &m_bitmap_grid_line[X[Fx+Nx]], bitmap_align(X[ARRAY_SIZE(X)-1]-X[Fx+Nx]));
                }

                r = X[Fx+Nx]-r0;
                if (m_circleCoordinates[scanLine].x > r) {
                    // print circle in right frame columns only
                    for (n = 0; n < m_circleCoordinates[scanLine].len; n++) {
                        VGA_PIXELINROW(dest, r0 + m_circleCoordinates[scanLine].x + n) = m_fgcolor;
                    }
                }
            }
        }

        if (scanLine >= H) {
            // signal end of visible screen area
            xSemaphoreGiveFromISR(m_verticalSyncSemaphore, nullptr);
        }
    }

    static void drawScanlineIRQ(void* arg, uint8_t* dest, int scanLine)
    {
        // forward to the instance method
        TestScreen::Impl* self = static_cast<TestScreen::Impl*>(arg);
        self->drawScanlineIRQ(dest, scanLine);
    }
};


TestScreen::TestScreen(char const* modeline) noexcept
  : m_impl(std::unique_ptr<TestScreen::Impl>(new TestScreen::Impl()))
{
    if (modeline) {
        setResolution(modeline);
    }
}

TestScreen::~TestScreen()
{
}

bool TestScreen::setResolution(char const* modeline)
{
    fabgl::VGATimings timings;
    if (fabgl::VGABaseController::convertModelineToTimings(modeline, &timings)) {
        m_impl->setResolution(timings);
        return true;
    }
    return false;
}

void TestScreen::setResolution(fabgl::VGATimings const& timings) { m_impl->setResolution(timings); }
void TestScreen::shrinkScreen(int dX, int dY) { m_impl->vga.shrinkScreen(dX, dY); }
void TestScreen::moveScreen(int dX, int dY) { m_impl->vga.moveScreen(dX, dY); }
unsigned int TestScreen::getScreenWidth() const { return m_impl->vga.getScreenWidth(); }
unsigned int TestScreen::getScreenHeight() const { return m_impl->vga.getScreenHeight(); }
void TestScreen::setBGColor(RGB222 color) { m_impl->setBGColor(color); }
void TestScreen::setFGColor(RGB222 color) { m_impl->setFGColor(color); }
fabgl::VGATimings const& TestScreen::getResolutionTimings() { return *m_impl->vga.getResolutionTimings(); }
SemaphoreHandle_t TestScreen::getVerticalSyncSemaphore() const { return m_impl->m_verticalSyncSemaphore; }
