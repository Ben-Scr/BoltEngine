namespace Bolt
{
    /// <summary>
    /// Base class for user C# scripts. Subclass and implement Start()/Update()/OnDestroy().
    /// </summary>
    public abstract class BoltScript
    {
        public Entity Entity { get; internal set; } = Entity.Invalid;

        internal void _SetEntityID(ulong id)
        {
            Entity = new Entity(id);
        }

        protected Vector2 Position
        {
            get => Entity.Position;
            set => Entity.Position = value;
        }

        protected float Rotation
        {
            get => Entity.Rotation;
            set => Entity.Rotation = value;
        }

        protected Entity? FindEntity(string name) => Entity.FindByName(name);
        protected T? GetComponent<T>() where T : Component, new() => Entity.GetComponent<T>();
    }
}
