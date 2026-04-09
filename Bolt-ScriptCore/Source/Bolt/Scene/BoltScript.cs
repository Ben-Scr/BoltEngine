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

        protected Entity? FindEntity(string name) => Entity.FindByName(name);
        protected T? GetComponent<T>() where T : Component, new() => Entity.GetComponent<T>();

        /// <summary>Create a new empty entity with the given name.</summary>
        protected Entity Create(string name = "") => Entity.Create(name);

        /// <summary>Clone an existing entity with all its components (prefab instantiation).</summary>
        protected Entity Create(Entity source) => Entity.Create(source);
    }
}
