namespace Bolt
{
    /// <summary>
    /// Logging interface that forwards messages to the native Bolt logging system (spdlog).
    /// </summary>
    public static class Log
    {
        public static void Trace(string message) => InternalCalls.Log_Trace(message);
        public static void Trace(object obj) => Trace(obj?.ToString() ?? "null");

        public static void Info(string message) => InternalCalls.Log_Info(message);
        public static void Info(object obj) => Info(obj?.ToString() ?? "null");

        public static void Warn(string message) => InternalCalls.Log_Warn(message);
        public static void Warn(object obj) => Warn(obj?.ToString() ?? "null");

        public static void Error(string message) => InternalCalls.Log_Error(message);
        public static void Error(object obj) => Error(obj?.ToString() ?? "null");
    }
}
