namespace Bolt
{
    /// <summary>
    /// Provides static access to application-level runtime information
    /// such as timing, screen dimensions, and lifecycle control.
    /// </summary>
    public static class Application
    {
        /// <summary>
        /// Current screen/window width in pixels.
        /// </summary>
        public static int ScreenWidth => InternalCalls.Application_GetScreenWidth();

        /// <summary>
        /// Current screen/window height in pixels.
        /// </summary>
        public static int ScreenHeight => InternalCalls.Application_GetScreenHeight();

        /// <summary>
        /// Current screen aspect ratio (width / height).
        /// </summary>
        public static float AspectRatio => ScreenHeight > 0 ? (float)ScreenWidth / ScreenHeight : 1.0f;
    }
}
