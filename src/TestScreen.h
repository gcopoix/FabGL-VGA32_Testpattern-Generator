#include <fabgl.h>
#include <memory>

/**
 * @brief Draw/Show a retro-like TV test screen. All FabGL resolutions supported with 64 colors.
 */
class TestScreen
{
public:
    /**
     * @brief Initializes a new instance of TestScreen
     *
     * @param modeline Optional ModeLine. If given, TestScreen will directly setup this resolution.
     *
     * Example:
     *     TestScreen testScreen(VGA_640x480_60Hz);
     */
    TestScreen(char const* modeline = nullptr) noexcept;

    virtual ~TestScreen();

    /**
     * @brief Sets the resolution of the TestScreen
     *
     * @param modeline ModeLine with resolution/timing information to set.
     * 
     * Example:
     *     TestScreen testScreen();
     *     testScreen.setResolution(VGA_640x480_60Hz);
     */
    bool setResolution(char const* modeline);
    /**
     * @brief Sets the resolution of the TestScreen
     *
     * @param timings Resolution/Timing information to set.
     */
    void setResolution(fabgl::VGATimings const& timings);

    /**
     * @brief Get current resolution timing
     *
     * @return Current resolution/timing information
     */
    fabgl::VGATimings const& getResolutionTimings();

  /**
   * @brief Reduce or expand screen size by the specified horizontal and vertical offset.
   *
   * @param dX Horizontal change in pixels. > 0 shrinks, < 0 expands.
   * @param dY Vertical change in pixels. > 0 shrinks, < 0 expands.
   */
    void shrinkScreen(int dX, int dY);

    /**
     * @brief Move screen by specified horizontal and vertical offset.
     *
     * @param dX Horizontal offset in pixels. < 0 goes left, > 0 goes right.
     * @param dY Vertical offset in pixels. < 0 goes up, > 0 goes down.
     */
    void moveScreen(int dX, int dY);

    /**
     * @brief Get the current screen width in pixels.
     *
     * @return Screen width in pixels.
     */
    unsigned int getScreenWidth() const;

    /**
     * @brief Get the current screen height in pixels.
     *
     * @return Screen height in pixels.
     */
    unsigned int getScreenHeight() const;

    /**
     * @brief Set the background color of the screen.
     *
     * @param color Background color to set in native RGB222 format
     */
    void setBGColor(RGB222 color);

    /**
     * @brief Set the foreground color of the screen.
     *
     * @param color Foreground color to set in native RGB222 format
     */
    void setFGColor(RGB222 color);

    /**
     * @brief Get the vertical sync semaphore.
     *
     * @note This semaphore can be used to synchronize with the vertical sync signal.
     * 
     * Example:
     *     TestScreen testScreen;
     *     ...
     *     SemaphoreHandle_t vsync = testScreen.getVerticalSyncSemaphore();
     *     ...
     *     while(true) {
     *         // Wait for vertical sync
     *         xSemaphoreTake(vsync, portMAX_DELAY);
     * 
     *         ...
     *     }
     *     
     * @return Vertical sync semaphore handle.
     */
    SemaphoreHandle_t getVerticalSyncSemaphore() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;

    static constexpr size_t Fx = 2;   // additional outer frame columns for the grid
    static constexpr size_t Fy = 2;   // additional outer frame rows for the grid
    static constexpr size_t Ny = 9;   // number of inner rows of test screen
    static constexpr size_t Nx = 8;   // number of inner columns of test screen (= number of colors in drawColorBarLine())
};
