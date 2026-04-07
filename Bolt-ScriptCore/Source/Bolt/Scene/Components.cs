namespace Bolt
{
    public class NameComponent : Component
    {
        public string Name
        {
            get => InternalCalls.NameComponent_GetName(Entity.ID);
            set => InternalCalls.NameComponent_SetName(Entity.ID, value);
        }
    }

    public class Transform2DComponent : Component
    {
        public Vector2 Position
        {
            get
            {
                InternalCalls.Transform2D_GetPosition(Entity.ID, out float x, out float y);
                return new Vector2(x, y);
            }
            set => InternalCalls.Transform2D_SetPosition(Entity.ID, value.X, value.Y);
        }

        public float Rotation
        {
            get => InternalCalls.Transform2D_GetRotation(Entity.ID);
            set => InternalCalls.Transform2D_SetRotation(Entity.ID, value);
        }

        public Vector2 Scale
        {
            get
            {
                InternalCalls.Transform2D_GetScale(Entity.ID, out float x, out float y);
                return new Vector2(x, y);
            }
            set => InternalCalls.Transform2D_SetScale(Entity.ID, value.X, value.Y);
        }
    }

    public class SpriteRendererComponent : Component
    {
        public Vector4 Color
        {
            get
            {
                InternalCalls.SpriteRenderer_GetColor(Entity.ID, out float r, out float g, out float b, out float a);
                return new Vector4(r, g, b, a);
            }
            set => InternalCalls.SpriteRenderer_SetColor(Entity.ID, value.X, value.Y, value.Z, value.W);
        }
    }

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
    }

    public class Rigidbody2DComponent : Component
    {
        public Vector2 LinearVelocity
        {
            get
            {
                InternalCalls.Rigidbody2D_GetLinearVelocity(Entity.ID, out float x, out float y);
                return new Vector2(x, y);
            }
            set => InternalCalls.Rigidbody2D_SetLinearVelocity(Entity.ID, value.X, value.Y);
        }

        public float AngularVelocity
        {
            get => InternalCalls.Rigidbody2D_GetAngularVelocity(Entity.ID);
            set => InternalCalls.Rigidbody2D_SetAngularVelocity(Entity.ID, value);
        }

        public void ApplyForce(Vector2 force, bool wake = true)
            => InternalCalls.Rigidbody2D_ApplyForce(Entity.ID, force.X, force.Y, wake);

        public void ApplyImpulse(Vector2 impulse, bool wake = true)
            => InternalCalls.Rigidbody2D_ApplyImpulse(Entity.ID, impulse.X, impulse.Y, wake);
    }
}
