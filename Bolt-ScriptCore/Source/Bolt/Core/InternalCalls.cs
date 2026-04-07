using System;
using System.Runtime.InteropServices;
using System.Text;
using Bolt.Hosting;

namespace Bolt
{
    internal static unsafe class InternalCalls
    {
        // ── Application ─────────────────────────────────────────────────

        internal static float Application_GetDeltaTime()
            => NativeCallbacks.Bindings.Application_GetDeltaTime();

        internal static float Application_GetElapsedTime()
            => NativeCallbacks.Bindings.Application_GetElapsedTime();

        internal static int Application_GetScreenWidth()
            => NativeCallbacks.Bindings.Application_GetScreenWidth();

        internal static int Application_GetScreenHeight()
            => NativeCallbacks.Bindings.Application_GetScreenHeight();

        // ── Log ─────────────────────────────────────────────────────────

        internal static void Log_Trace(string message)
        {
            int len = Encoding.UTF8.GetByteCount(message);
            Span<byte> buf = len <= 512 ? stackalloc byte[len + 1] : new byte[len + 1];
            Encoding.UTF8.GetBytes(message, buf);
            buf[len] = 0;
            fixed (byte* ptr = buf)
                NativeCallbacks.Bindings.Log_Trace(ptr);
        }

        internal static void Log_Info(string message)
        {
            int len = Encoding.UTF8.GetByteCount(message);
            Span<byte> buf = len <= 512 ? stackalloc byte[len + 1] : new byte[len + 1];
            Encoding.UTF8.GetBytes(message, buf);
            buf[len] = 0;
            fixed (byte* ptr = buf)
                NativeCallbacks.Bindings.Log_Info(ptr);
        }

        internal static void Log_Warn(string message)
        {
            int len = Encoding.UTF8.GetByteCount(message);
            Span<byte> buf = len <= 512 ? stackalloc byte[len + 1] : new byte[len + 1];
            Encoding.UTF8.GetBytes(message, buf);
            buf[len] = 0;
            fixed (byte* ptr = buf)
                NativeCallbacks.Bindings.Log_Warn(ptr);
        }

        internal static void Log_Error(string message)
        {
            int len = Encoding.UTF8.GetByteCount(message);
            Span<byte> buf = len <= 512 ? stackalloc byte[len + 1] : new byte[len + 1];
            Encoding.UTF8.GetBytes(message, buf);
            buf[len] = 0;
            fixed (byte* ptr = buf)
                NativeCallbacks.Bindings.Log_Error(ptr);
        }

        // ── Input ───────────────────────────────────────────────────────

        internal static bool Input_GetKey(int keyCode)
            => NativeCallbacks.Bindings.Input_GetKey(keyCode) != 0;

        internal static bool Input_GetKeyDown(int keyCode)
            => NativeCallbacks.Bindings.Input_GetKeyDown(keyCode) != 0;

        internal static bool Input_GetKeyUp(int keyCode)
            => NativeCallbacks.Bindings.Input_GetKeyUp(keyCode) != 0;

        internal static bool Input_GetMouseButton(int button)
            => NativeCallbacks.Bindings.Input_GetMouseButton(button) != 0;

        internal static bool Input_GetMouseButtonDown(int button)
            => NativeCallbacks.Bindings.Input_GetMouseButtonDown(button) != 0;

        internal static void Input_GetMousePosition(out float x, out float y)
        {
            float ox, oy;
            NativeCallbacks.Bindings.Input_GetMousePosition(&ox, &oy);
            x = ox; y = oy;
        }

        // ── Entity ──────────────────────────────────────────────────────

        internal static bool Entity_IsValid(ulong entityID)
            => NativeCallbacks.Bindings.Entity_IsValid(entityID) != 0;

        internal static ulong Entity_FindByName(string name)
        {
            int len = Encoding.UTF8.GetByteCount(name);
            Span<byte> buf = len <= 256 ? stackalloc byte[len + 1] : new byte[len + 1];
            Encoding.UTF8.GetBytes(name, buf);
            buf[len] = 0;
            fixed (byte* ptr = buf)
                return NativeCallbacks.Bindings.Entity_FindByName(ptr);
        }

        internal static void Entity_Destroy(ulong entityID)
            => NativeCallbacks.Bindings.Entity_Destroy(entityID);

        // ── NameComponent ───────────────────────────────────────────────

        internal static string NameComponent_GetName(ulong entityID)
        {
            byte* ptr = NativeCallbacks.Bindings.NameComponent_GetName(entityID);
            return Marshal.PtrToStringUTF8((IntPtr)ptr) ?? "";
        }

        internal static void NameComponent_SetName(ulong entityID, string name)
        {
            int len = Encoding.UTF8.GetByteCount(name);
            Span<byte> buf = len <= 256 ? stackalloc byte[len + 1] : new byte[len + 1];
            Encoding.UTF8.GetBytes(name, buf);
            buf[len] = 0;
            fixed (byte* ptr = buf)
                NativeCallbacks.Bindings.NameComponent_SetName(entityID, ptr);
        }

        // ── Transform2DComponent ────────────────────────────────────────

        internal static void Transform2D_GetPosition(ulong entityID, out float x, out float y)
        {
            float ox, oy;
            NativeCallbacks.Bindings.Transform2D_GetPosition(entityID, &ox, &oy);
            x = ox; y = oy;
        }

        internal static void Transform2D_SetPosition(ulong entityID, float x, float y)
            => NativeCallbacks.Bindings.Transform2D_SetPosition(entityID, x, y);

        internal static float Transform2D_GetRotation(ulong entityID)
            => NativeCallbacks.Bindings.Transform2D_GetRotation(entityID);

        internal static void Transform2D_SetRotation(ulong entityID, float rotation)
            => NativeCallbacks.Bindings.Transform2D_SetRotation(entityID, rotation);

        internal static void Transform2D_GetScale(ulong entityID, out float x, out float y)
        {
            float ox, oy;
            NativeCallbacks.Bindings.Transform2D_GetScale(entityID, &ox, &oy);
            x = ox; y = oy;
        }

        internal static void Transform2D_SetScale(ulong entityID, float x, float y)
            => NativeCallbacks.Bindings.Transform2D_SetScale(entityID, x, y);

        // ── SpriteRendererComponent ─────────────────────────────────────

        internal static void SpriteRenderer_GetColor(ulong entityID, out float r, out float g, out float b, out float a)
        {
            float or2, og, ob, oa;
            NativeCallbacks.Bindings.SpriteRenderer_GetColor(entityID, &or2, &og, &ob, &oa);
            r = or2; g = og; b = ob; a = oa;
        }

        internal static void SpriteRenderer_SetColor(ulong entityID, float r, float g, float b, float a)
            => NativeCallbacks.Bindings.SpriteRenderer_SetColor(entityID, r, g, b, a);

        // ── Camera2DComponent ───────────────────────────────────────────

        internal static float Camera2D_GetOrthographicSize(ulong entityID)
            => NativeCallbacks.Bindings.Camera2D_GetOrthographicSize(entityID);

        internal static void Camera2D_SetOrthographicSize(ulong entityID, float size)
            => NativeCallbacks.Bindings.Camera2D_SetOrthographicSize(entityID, size);

        internal static float Camera2D_GetZoom(ulong entityID)
            => NativeCallbacks.Bindings.Camera2D_GetZoom(entityID);

        internal static void Camera2D_SetZoom(ulong entityID, float zoom)
            => NativeCallbacks.Bindings.Camera2D_SetZoom(entityID, zoom);

        // ── Rigidbody2DComponent ────────────────────────────────────────

        internal static void Rigidbody2D_ApplyForce(ulong entityID, float forceX, float forceY, bool wake)
            => NativeCallbacks.Bindings.Rigidbody2D_ApplyForce(entityID, forceX, forceY, wake ? 1 : 0);

        internal static void Rigidbody2D_ApplyImpulse(ulong entityID, float impulseX, float impulseY, bool wake)
            => NativeCallbacks.Bindings.Rigidbody2D_ApplyImpulse(entityID, impulseX, impulseY, wake ? 1 : 0);

        internal static void Rigidbody2D_GetLinearVelocity(ulong entityID, out float x, out float y)
        {
            float ox, oy;
            NativeCallbacks.Bindings.Rigidbody2D_GetLinearVelocity(entityID, &ox, &oy);
            x = ox; y = oy;
        }

        internal static void Rigidbody2D_SetLinearVelocity(ulong entityID, float x, float y)
            => NativeCallbacks.Bindings.Rigidbody2D_SetLinearVelocity(entityID, x, y);

        internal static float Rigidbody2D_GetAngularVelocity(ulong entityID)
            => NativeCallbacks.Bindings.Rigidbody2D_GetAngularVelocity(entityID);

        internal static void Rigidbody2D_SetAngularVelocity(ulong entityID, float velocity)
            => NativeCallbacks.Bindings.Rigidbody2D_SetAngularVelocity(entityID, velocity);

        // ── Physics2D ───────────────────────────────────────────────────

        internal static bool Physics2D_Raycast(float originX, float originY, float dirX, float dirY, float distance,
            out ulong hitEntityID, out float hitX, out float hitY, out float hitNormalX, out float hitNormalY)
        {
            ulong eid;
            float hx, hy, hnx, hny;
            int result = NativeCallbacks.Bindings.Physics2D_Raycast(
                originX, originY, dirX, dirY, distance,
                &eid, &hx, &hy, &hnx, &hny);
            hitEntityID = eid;
            hitX = hx; hitY = hy;
            hitNormalX = hnx; hitNormalY = hny;
            return result != 0;
        }
    }
}
