namespace Bolt
{
    /// <summary>
    /// Predefined color constants as Vector4 (RGBA, 0-1 range).
    /// Matches the color representation used by Bolt's SpriteRendererComponent.
    /// </summary>
    public static class Color
    {
        public static readonly Vector4 White   = new(1.0f, 1.0f, 1.0f, 1.0f);
        public static readonly Vector4 Black   = new(0.0f, 0.0f, 0.0f, 1.0f);
        public static readonly Vector4 Red     = new(1.0f, 0.0f, 0.0f, 1.0f);
        public static readonly Vector4 Green   = new(0.0f, 1.0f, 0.0f, 1.0f);
        public static readonly Vector4 Blue    = new(0.0f, 0.0f, 1.0f, 1.0f);
        public static readonly Vector4 Yellow  = new(1.0f, 1.0f, 0.0f, 1.0f);
        public static readonly Vector4 Cyan    = new(0.0f, 1.0f, 1.0f, 1.0f);
        public static readonly Vector4 Magenta = new(1.0f, 0.0f, 1.0f, 1.0f);
        public static readonly Vector4 Gray    = new(0.5f, 0.5f, 0.5f, 1.0f);
        public static readonly Vector4 Clear   = new(0.0f, 0.0f, 0.0f, 0.0f);
    }
}
