using System.Runtime.InteropServices;

namespace Bolt.Hosting
{
    /// <summary>
    /// Layout must match the C++ NativeBindings struct exactly (Sequential, blittable).
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal unsafe struct NativeBindingsStruct
    {
        public delegate* unmanaged<float> Application_GetDeltaTime;
        public delegate* unmanaged<float> Application_GetElapsedTime;
        public delegate* unmanaged<int> Application_GetScreenWidth;
        public delegate* unmanaged<int> Application_GetScreenHeight;

        public delegate* unmanaged<byte*, void> Log_Trace;
        public delegate* unmanaged<byte*, void> Log_Info;
        public delegate* unmanaged<byte*, void> Log_Warn;
        public delegate* unmanaged<byte*, void> Log_Error;

        public delegate* unmanaged<int, int> Input_GetKey;
        public delegate* unmanaged<int, int> Input_GetKeyDown;
        public delegate* unmanaged<int, int> Input_GetKeyUp;
        public delegate* unmanaged<int, int> Input_GetMouseButton;
        public delegate* unmanaged<int, int> Input_GetMouseButtonDown;
        public delegate* unmanaged<float*, float*, void> Input_GetMousePosition;

        public delegate* unmanaged<ulong, int> Entity_IsValid;
        public delegate* unmanaged<byte*, ulong> Entity_FindByName;
        public delegate* unmanaged<ulong, void> Entity_Destroy;

        public delegate* unmanaged<ulong, byte*> NameComponent_GetName;
        public delegate* unmanaged<ulong, byte*, void> NameComponent_SetName;

        public delegate* unmanaged<ulong, float*, float*, void> Transform2D_GetPosition;
        public delegate* unmanaged<ulong, float, float, void> Transform2D_SetPosition;
        public delegate* unmanaged<ulong, float> Transform2D_GetRotation;
        public delegate* unmanaged<ulong, float, void> Transform2D_SetRotation;
        public delegate* unmanaged<ulong, float*, float*, void> Transform2D_GetScale;
        public delegate* unmanaged<ulong, float, float, void> Transform2D_SetScale;

        public delegate* unmanaged<ulong, float*, float*, float*, float*, void> SpriteRenderer_GetColor;
        public delegate* unmanaged<ulong, float, float, float, float, void> SpriteRenderer_SetColor;

        public delegate* unmanaged<ulong, float> Camera2D_GetOrthographicSize;
        public delegate* unmanaged<ulong, float, void> Camera2D_SetOrthographicSize;
        public delegate* unmanaged<ulong, float> Camera2D_GetZoom;
        public delegate* unmanaged<ulong, float, void> Camera2D_SetZoom;

        public delegate* unmanaged<ulong, float, float, int, void> Rigidbody2D_ApplyForce;
        public delegate* unmanaged<ulong, float, float, int, void> Rigidbody2D_ApplyImpulse;
        public delegate* unmanaged<ulong, float*, float*, void> Rigidbody2D_GetLinearVelocity;
        public delegate* unmanaged<ulong, float, float, void> Rigidbody2D_SetLinearVelocity;
        public delegate* unmanaged<ulong, float> Rigidbody2D_GetAngularVelocity;
        public delegate* unmanaged<ulong, float, void> Rigidbody2D_SetAngularVelocity;

        public delegate* unmanaged<float, float, float, float, float, ulong*, float*, float*, float*, float*, int> Physics2D_Raycast;
    }

    internal static unsafe class NativeCallbacks
    {
        internal static NativeBindingsStruct Bindings;

        internal static void SetFrom(NativeBindingsStruct* native)
        {
            Bindings = *native;
        }
    }
}
