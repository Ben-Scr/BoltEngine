namespace Bolt
{
    /// <summary>
    /// Base class for all managed component wrappers.
    /// Each component holds a reference to its owning Entity, which is used
    /// to route property access through InternalCalls to the native ECS data.
    /// </summary>
    public abstract class Component
    {
        public Entity Entity { get; internal set; } = Entity.Invalid;
    }
}
