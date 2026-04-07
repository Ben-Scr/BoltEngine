namespace Bolt
{
    public enum MouseButton
    {
        Left = 0,
        Right = 1,
        Middle = 2
    }

    /// <summary>
    /// Provides static access to keyboard and mouse input state.
    /// Maps to the native Bolt InputManager.
    /// </summary>
    public static class Input
    {
        /// <summary>
        /// Returns true every frame the key is held down.
        /// </summary>
        public static bool GetKey(KeyCode key) => InternalCalls.Input_GetKey((int)key);

        /// <summary>
        /// Returns true only on the frame the key was first pressed.
        /// </summary>
        public static bool GetKeyDown(KeyCode key) => InternalCalls.Input_GetKeyDown((int)key);

        /// <summary>
        /// Returns true only on the frame the key was released.
        /// </summary>
        public static bool GetKeyUp(KeyCode key) => InternalCalls.Input_GetKeyUp((int)key);

        /// <summary>
        /// Returns true every frame the mouse button is held down.
        /// </summary>
        public static bool GetMouseButton(MouseButton button) => InternalCalls.Input_GetMouseButton((int)button);

        /// <summary>
        /// Returns true only on the frame the mouse button was first pressed.
        /// </summary>
        public static bool GetMouseButtonDown(MouseButton button) => InternalCalls.Input_GetMouseButtonDown((int)button);

        /// <summary>
        /// Returns the current mouse position in screen coordinates.
        /// </summary>
        public static Vector2 GetMousePosition()
        {
            InternalCalls.Input_GetMousePosition(out float x, out float y);
            return new Vector2(x, y);
        }
    }
}
