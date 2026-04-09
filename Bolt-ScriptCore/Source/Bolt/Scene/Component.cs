namespace Bolt
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; } = Entity.Invalid;
    }
}
