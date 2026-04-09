namespace Bolt
{
    // ── NameComponent ───────────────────────────────────────────────────

    public class NameComponent : Component
    {
        public string Name
        {
            get => InternalCalls.NameComponent_GetName(Entity.ID);
            set => InternalCalls.NameComponent_SetName(Entity.ID, value);
        }
    }

    // ── Transform2DComponent ────────────────────────────────────────────

    public class Transform2DComponent : Component
    {
        public Vector2 Position
        {
            get { InternalCalls.Transform2D_GetPosition(Entity.ID, out float x, out float y); return new Vector2(x, y); }
            set => InternalCalls.Transform2D_SetPosition(Entity.ID, value.X, value.Y);
        }

        public float Rotation
        {
            get => InternalCalls.Transform2D_GetRotation(Entity.ID);
            set => InternalCalls.Transform2D_SetRotation(Entity.ID, value);
        }

        public float RotationDegrees
        {
            get => Rotation * Mathf.Rad2Deg;
            set => Rotation = value * Mathf.Deg2Rad;
        }

        public Vector2 Scale
        {
            get { InternalCalls.Transform2D_GetScale(Entity.ID, out float x, out float y); return new Vector2(x, y); }
            set => InternalCalls.Transform2D_SetScale(Entity.ID, value.X, value.Y);
        }
    }

    // ── SpriteRendererComponent ─────────────────────────────────────────

    public class SpriteRendererComponent : Component
    {
        public Vector4 Color
        {
            get { InternalCalls.SpriteRenderer_GetColor(Entity.ID, out float r, out float g, out float b, out float a); return new Color(r, g, b, a); }
            set => InternalCalls.SpriteRenderer_SetColor(Entity.ID, value.X, value.Y, value.Z, value.W);
        }

        public int SortingOrder
        {
            get => InternalCalls.SpriteRenderer_GetSortingOrder(Entity.ID);
            set => InternalCalls.SpriteRenderer_SetSortingOrder(Entity.ID, value);
        }

        public int SortingLayer
        {
            get => InternalCalls.SpriteRenderer_GetSortingLayer(Entity.ID);
            set => InternalCalls.SpriteRenderer_SetSortingLayer(Entity.ID, value);
        }
    }

    // ── Camera2DComponent ───────────────────────────────────────────────

    public class Camera2DComponent : Component
    {
        public float OrthographicSize
        {
            get => InternalCalls.Camera2D_GetOrthographicSize(Entity.ID);
            set => InternalCalls.Camera2D_SetOrthographicSize(Entity.ID, value);
        }

        public float Zoom
        {
            get => InternalCalls.Camera2D_GetZoom(Entity.ID);
            set => InternalCalls.Camera2D_SetZoom(Entity.ID, value);
        }

        public Vector4 ClearColor
        {
            get { InternalCalls.Camera2D_GetClearColor(Entity.ID, out float r, out float g, out float b, out float a); return new Color(r, g, b, a); }
            set => InternalCalls.Camera2D_SetClearColor(Entity.ID, value.X, value.Y, value.Z, value.W);
        }

        public Vector2 ScreenToWorld(Vector2 screenPos)
        {
            InternalCalls.Camera2D_ScreenToWorld(Entity.ID, screenPos.X, screenPos.Y, out float wx, out float wy);
            return new Vector2(wx, wy);
        }

        public float ViewportWidth => InternalCalls.Camera2D_GetViewportWidth(Entity.ID);
        public float ViewportHeight => InternalCalls.Camera2D_GetViewportHeight(Entity.ID);
    }

    // ── Rigidbody2DComponent ────────────────────────────────────────────

    public enum BodyType { Static = 0, Kinematic = 1, Dynamic = 2 }

    public class Rigidbody2DComponent : Component
    {
        public Vector2 LinearVelocity
        {
            get { InternalCalls.Rigidbody2D_GetLinearVelocity(Entity.ID, out float x, out float y); return new Vector2(x, y); }
            set => InternalCalls.Rigidbody2D_SetLinearVelocity(Entity.ID, value.X, value.Y);
        }

        public float AngularVelocity
        {
            get => InternalCalls.Rigidbody2D_GetAngularVelocity(Entity.ID);
            set => InternalCalls.Rigidbody2D_SetAngularVelocity(Entity.ID, value);
        }

        public BodyType BodyType
        {
            get => (BodyType)InternalCalls.Rigidbody2D_GetBodyType(Entity.ID);
            set => InternalCalls.Rigidbody2D_SetBodyType(Entity.ID, (int)value);
        }

        public float GravityScale
        {
            get => InternalCalls.Rigidbody2D_GetGravityScale(Entity.ID);
            set => InternalCalls.Rigidbody2D_SetGravityScale(Entity.ID, value);
        }

        public float Mass
        {
            get => InternalCalls.Rigidbody2D_GetMass(Entity.ID);
            set => InternalCalls.Rigidbody2D_SetMass(Entity.ID, value);
        }

        public void ApplyForce(Vector2 force, bool wake = true)
            => InternalCalls.Rigidbody2D_ApplyForce(Entity.ID, force.X, force.Y, wake);

        public void ApplyImpulse(Vector2 impulse, bool wake = true)
            => InternalCalls.Rigidbody2D_ApplyImpulse(Entity.ID, impulse.X, impulse.Y, wake);
    }

    // ── BoxCollider2DComponent ──────────────────────────────────────────

    public class BoxCollider2DComponent : Component
    {
        public Vector2 Scale
        {
            get { InternalCalls.BoxCollider2D_GetScale(Entity.ID, out float x, out float y); return new Vector2(x, y); }
        }

        public Vector2 Center
        {
            get { InternalCalls.BoxCollider2D_GetCenter(Entity.ID, out float x, out float y); return new Vector2(x, y); }
        }

        public bool Enabled
        {
            set => InternalCalls.BoxCollider2D_SetEnabled(Entity.ID, value);
        }
    }

    // ── AudioSourceComponent ────────────────────────────────────────────

    public class AudioSourceComponent : Component
    {
        public float Volume
        {
            get => InternalCalls.AudioSource_GetVolume(Entity.ID);
            set => InternalCalls.AudioSource_SetVolume(Entity.ID, value);
        }

        public float Pitch
        {
            get => InternalCalls.AudioSource_GetPitch(Entity.ID);
            set => InternalCalls.AudioSource_SetPitch(Entity.ID, value);
        }

        public bool Loop
        {
            get => InternalCalls.AudioSource_GetLoop(Entity.ID);
            set => InternalCalls.AudioSource_SetLoop(Entity.ID, value);
        }

        public bool IsPlaying => InternalCalls.AudioSource_IsPlaying(Entity.ID);

        public void Play() => InternalCalls.AudioSource_Play(Entity.ID);
        public void Pause() => InternalCalls.AudioSource_Pause(Entity.ID);
        public void Stop() => InternalCalls.AudioSource_Stop(Entity.ID);
        public void Resume() => InternalCalls.AudioSource_Resume(Entity.ID);
    }

    // ── Bolt-Physics Components ─────────────────────────────────────────
    // These use the lightweight Bolt-Physics engine (AABB-based collision).
    // For full physics (rotation, friction, CCD), use Rigidbody2DComponent
    // and BoxCollider2DComponent instead (Box2D-backed).

    public enum BoltBodyType { Static = 0, Dynamic = 1, Kinematic = 2 }

    public class BoltBody2DComponent : Component
    {
        public BoltBodyType BodyType
        {
            get => (BoltBodyType)InternalCalls.BoltBody2D_GetBodyType(Entity.ID);
            set => InternalCalls.BoltBody2D_SetBodyType(Entity.ID, (int)value);
        }

        public float Mass
        {
            get => InternalCalls.BoltBody2D_GetMass(Entity.ID);
            set => InternalCalls.BoltBody2D_SetMass(Entity.ID, value);
        }

        public bool UseGravity
        {
            get => InternalCalls.BoltBody2D_GetUseGravity(Entity.ID);
            set => InternalCalls.BoltBody2D_SetUseGravity(Entity.ID, value);
        }

        public Vector2 Velocity
        {
            get { InternalCalls.BoltBody2D_GetVelocity(Entity.ID, out float x, out float y); return new Vector2(x, y); }
            set => InternalCalls.BoltBody2D_SetVelocity(Entity.ID, value.X, value.Y);
        }
    }

    public class BoltBoxCollider2DComponent : Component
    {
        public Vector2 HalfExtents
        {
            get { InternalCalls.BoltBoxCollider2D_GetHalfExtents(Entity.ID, out float x, out float y); return new Vector2(x, y); }
            set => InternalCalls.BoltBoxCollider2D_SetHalfExtents(Entity.ID, value.X, value.Y);
        }
    }

    public class BoltCircleCollider2DComponent : Component
    {
        public float Radius
        {
            get => InternalCalls.BoltCircleCollider2D_GetRadius(Entity.ID);
            set => InternalCalls.BoltCircleCollider2D_SetRadius(Entity.ID, value);
        }
    }
}
