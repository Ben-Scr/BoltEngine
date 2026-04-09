using System.Runtime.InteropServices;

namespace Bolt.Hosting
{
    /// <summary>
    /// Layout must match the C++ NativeBindings struct exactly (Sequential, blittable).
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal unsafe struct NativeBindingsStruct
    {
        // ── Application ──────────────────────────────────────────────
        public delegate* unmanaged<float> Application_GetDeltaTime;
        public delegate* unmanaged<float> Application_GetElapsedTime;
        public delegate* unmanaged<int> Application_GetScreenWidth;
        public delegate* unmanaged<int> Application_GetScreenHeight;

        // ── Log ──────────────────────────────────────────────────────
        public delegate* unmanaged<byte*, void> Log_Trace;
        public delegate* unmanaged<byte*, void> Log_Info;
        public delegate* unmanaged<byte*, void> Log_Warn;
        public delegate* unmanaged<byte*, void> Log_Error;

        // ── Input ────────────────────────────────────────────────────
        public delegate* unmanaged<int, int> Input_GetKey;
        public delegate* unmanaged<int, int> Input_GetKeyDown;
        public delegate* unmanaged<int, int> Input_GetKeyUp;
        public delegate* unmanaged<int, int> Input_GetMouseButton;
        public delegate* unmanaged<int, int> Input_GetMouseButtonDown;
        public delegate* unmanaged<float*, float*, void> Input_GetMousePosition;

        // ── Entity ───────────────────────────────────────────────────
        public delegate* unmanaged<ulong, int> Entity_IsValid;
        public delegate* unmanaged<byte*, ulong> Entity_FindByName;
        public delegate* unmanaged<ulong, void> Entity_Destroy;
        public delegate* unmanaged<byte*, ulong> Entity_Create;
        public delegate* unmanaged<ulong, ulong> Entity_Clone;
        public delegate* unmanaged<ulong, byte*, int> Entity_HasComponent;
        public delegate* unmanaged<ulong, byte*, int> Entity_AddComponent;
        public delegate* unmanaged<ulong, byte*, int> Entity_RemoveComponent;

        // ── NameComponent ────────────────────────────────────────────
        public delegate* unmanaged<ulong, byte*> NameComponent_GetName;
        public delegate* unmanaged<ulong, byte*, void> NameComponent_SetName;

        // ── Transform2D ──────────────────────────────────────────────
        public delegate* unmanaged<ulong, float*, float*, void> Transform2D_GetPosition;
        public delegate* unmanaged<ulong, float, float, void> Transform2D_SetPosition;
        public delegate* unmanaged<ulong, float> Transform2D_GetRotation;
        public delegate* unmanaged<ulong, float, void> Transform2D_SetRotation;
        public delegate* unmanaged<ulong, float*, float*, void> Transform2D_GetScale;
        public delegate* unmanaged<ulong, float, float, void> Transform2D_SetScale;

        // ── SpriteRenderer ───────────────────────────────────────────
        public delegate* unmanaged<ulong, float*, float*, float*, float*, void> SpriteRenderer_GetColor;
        public delegate* unmanaged<ulong, float, float, float, float, void> SpriteRenderer_SetColor;
        public delegate* unmanaged<ulong, int> SpriteRenderer_GetSortingOrder;
        public delegate* unmanaged<ulong, int, void> SpriteRenderer_SetSortingOrder;
        public delegate* unmanaged<ulong, int> SpriteRenderer_GetSortingLayer;
        public delegate* unmanaged<ulong, int, void> SpriteRenderer_SetSortingLayer;

        // ── Camera2D ─────────────────────────────────────────────────
        public delegate* unmanaged<ulong, float> Camera2D_GetOrthographicSize;
        public delegate* unmanaged<ulong, float, void> Camera2D_SetOrthographicSize;
        public delegate* unmanaged<ulong, float> Camera2D_GetZoom;
        public delegate* unmanaged<ulong, float, void> Camera2D_SetZoom;
        public delegate* unmanaged<ulong, float*, float*, float*, float*, void> Camera2D_GetClearColor;
        public delegate* unmanaged<ulong, float, float, float, float, void> Camera2D_SetClearColor;
        public delegate* unmanaged<ulong, float, float, float*, float*, void> Camera2D_ScreenToWorld;
        public delegate* unmanaged<ulong, float> Camera2D_GetViewportWidth;
        public delegate* unmanaged<ulong, float> Camera2D_GetViewportHeight;

        // ── Rigidbody2D ──────────────────────────────────────────────
        public delegate* unmanaged<ulong, float, float, int, void> Rigidbody2D_ApplyForce;
        public delegate* unmanaged<ulong, float, float, int, void> Rigidbody2D_ApplyImpulse;
        public delegate* unmanaged<ulong, float*, float*, void> Rigidbody2D_GetLinearVelocity;
        public delegate* unmanaged<ulong, float, float, void> Rigidbody2D_SetLinearVelocity;
        public delegate* unmanaged<ulong, float> Rigidbody2D_GetAngularVelocity;
        public delegate* unmanaged<ulong, float, void> Rigidbody2D_SetAngularVelocity;
        public delegate* unmanaged<ulong, int> Rigidbody2D_GetBodyType;
        public delegate* unmanaged<ulong, int, void> Rigidbody2D_SetBodyType;
        public delegate* unmanaged<ulong, float> Rigidbody2D_GetGravityScale;
        public delegate* unmanaged<ulong, float, void> Rigidbody2D_SetGravityScale;
        public delegate* unmanaged<ulong, float> Rigidbody2D_GetMass;
        public delegate* unmanaged<ulong, float, void> Rigidbody2D_SetMass;

        // ── BoxCollider2D ────────────────────────────────────────────
        public delegate* unmanaged<ulong, float*, float*, void> BoxCollider2D_GetScale;
        public delegate* unmanaged<ulong, float*, float*, void> BoxCollider2D_GetCenter;
        public delegate* unmanaged<ulong, int, void> BoxCollider2D_SetEnabled;

        // ── AudioSource ──────────────────────────────────────────────
        public delegate* unmanaged<ulong, void> AudioSource_Play;
        public delegate* unmanaged<ulong, void> AudioSource_Pause;
        public delegate* unmanaged<ulong, void> AudioSource_Stop;
        public delegate* unmanaged<ulong, void> AudioSource_Resume;
        public delegate* unmanaged<ulong, float> AudioSource_GetVolume;
        public delegate* unmanaged<ulong, float, void> AudioSource_SetVolume;
        public delegate* unmanaged<ulong, float> AudioSource_GetPitch;
        public delegate* unmanaged<ulong, float, void> AudioSource_SetPitch;
        public delegate* unmanaged<ulong, int> AudioSource_GetLoop;
        public delegate* unmanaged<ulong, int, void> AudioSource_SetLoop;
        public delegate* unmanaged<ulong, int> AudioSource_IsPlaying;

        // ── Bolt-Physics ─────────────────────────────────────────────
        public delegate* unmanaged<ulong, int> BoltBody2D_GetBodyType;
        public delegate* unmanaged<ulong, int, void> BoltBody2D_SetBodyType;
        public delegate* unmanaged<ulong, float> BoltBody2D_GetMass;
        public delegate* unmanaged<ulong, float, void> BoltBody2D_SetMass;
        public delegate* unmanaged<ulong, int> BoltBody2D_GetUseGravity;
        public delegate* unmanaged<ulong, int, void> BoltBody2D_SetUseGravity;
        public delegate* unmanaged<ulong, float*, float*, void> BoltBody2D_GetVelocity;
        public delegate* unmanaged<ulong, float, float, void> BoltBody2D_SetVelocity;
        public delegate* unmanaged<ulong, float*, float*, void> BoltBoxCollider2D_GetHalfExtents;
        public delegate* unmanaged<ulong, float, float, void> BoltBoxCollider2D_SetHalfExtents;
        public delegate* unmanaged<ulong, float> BoltCircleCollider2D_GetRadius;
        public delegate* unmanaged<ulong, float, void> BoltCircleCollider2D_SetRadius;

        // ── Gizmos ───────────────────────────────────────────────────
        public delegate* unmanaged<float, float, float, float, void> Gizmo_DrawLine;
        public delegate* unmanaged<float, float, float, float, float, void> Gizmo_DrawSquare;
        public delegate* unmanaged<float, float, float, int, void> Gizmo_DrawCircle;
        public delegate* unmanaged<float, float, float, float, void> Gizmo_SetColor;
        public delegate* unmanaged<float*, float*, float*, float*, void> Gizmo_GetColor;
        public delegate* unmanaged<float, void> Gizmo_SetLineWidth;

        // ── Physics2D ────────────────────────────────────────────────
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
