
namespace Bolt
{
    public static class Time
    {
        /// <summary>
        /// Time in seconds since the last frame.
        /// </summary>
        public static float DeltaTime => InternalCalls.Application_GetDeltaTime();

        /// <summary>
        /// Time in seconds since the application started.
        /// </summary>
        public static float ElapsedTime => InternalCalls.Application_GetElapsedTime();
    }
}
