
namespace Bolt
{
    public static class Time
    {
        public static float DeltaTime => InternalCalls.Application_GetDeltaTime();
        public static float UnscaledDeltaTime => 0.0f;
        public static float FixedDeltaTime => 0.0f;
        public static float FixedUnscaledDeltaTime => 0.0f;

        public static float ElapsedTime => InternalCalls.Application_GetElapsedTime();
    }
}
