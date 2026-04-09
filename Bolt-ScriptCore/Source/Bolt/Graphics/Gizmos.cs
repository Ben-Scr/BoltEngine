namespace Bolt
{
    public static class Gizmos
    {
        public static void SetColor(Color color)
            => InternalCalls.Gizmo_SetColor(color.R, color.G, color.B, color.A);

        public static Color GetColor()
        {
            InternalCalls.Gizmo_GetColor(out float r, out float g, out float b, out float a);
            return new Color(r, g, b, a);
        }

        public static void SetLineWidth(float width)
            => InternalCalls.Gizmo_SetLineWidth(width);

        public static void DrawLine(Vector2 start, Vector2 end)
            => InternalCalls.Gizmo_DrawLine(start.X, start.Y, end.X, end.Y);

        public static void DrawSquare(Vector2 center, Vector2 size, float degrees = 0.0f)
            => InternalCalls.Gizmo_DrawSquare(center.X, center.Y, size.X, size.Y, degrees);

        public static void DrawCircle(Vector2 center, float radius, int segments = 32)
            => InternalCalls.Gizmo_DrawCircle(center.X, center.Y, radius, segments);
    }
}
