using System;

namespace Bolt
{
    /// <summary>
    /// Provides static access to application-level runtime information
    /// such as timing, screen dimensions, and lifecycle control.
    /// </summary>
    public static class Application
    {
        public static Action OnApplicationQuit;
        public static Action<bool> OnWindowFocus;

        public static float TargetFrameRate
        {
            get => InternalCalls.Application_GetTargetFrameRate();
            set => InternalCalls.Application_SetTargetFrameRate(value);
        }

        public static int ScreenWidth => InternalCalls.Application_GetScreenWidth();
        public static int ScreenHeight => InternalCalls.Application_GetScreenHeight();
        public static float AspectRatio => ScreenHeight > 0 ? (float)ScreenWidth / ScreenHeight : 1.0f;

        /// <summary>
        /// Quits the application. Only works in build mode, ignored in the editor.
        /// </summary>
        public static void Quit() => InternalCalls.Application_Quit();
    }
}
