using System;
using System.Collections.Generic;

namespace Bolt
{
    public class Entity : IEquatable<Entity>
    {
        public static readonly Entity Invalid = new(0);

        private readonly Dictionary<Type, Component> m_ComponentCache = new();
        private Transform2DComponent? m_TransformComponent;

        protected Entity() { ID = 0; }
        internal Entity(ulong id) { ID = id; }

        public readonly ulong ID;

        public string Name => GetComponent<NameComponent>()?.Name ?? "";

        public Transform2DComponent Transform
        {
            get
            {
                if (m_TransformComponent == null)
                {
                    m_TransformComponent = GetComponent<Transform2DComponent>();
                    if (m_TransformComponent == null)
                        throw new InvalidOperationException("Entity does not have a Transform2DComponent");
                }
                return m_TransformComponent;
            }
        }

        public Vector2 Position
        {
            get => Transform.Position;
            set => Transform.Position = value;
        }

        public float Rotation
        {
            get => Transform.Rotation;
            set => Transform.Rotation = value;
        }

        public Vector2 Scale
        {
            get => Transform.Scale;
            set => Transform.Scale = value;
        }

        public T? GetComponent<T>() where T : Component, new()
        {
            Type type = typeof(T);
            if (m_ComponentCache.TryGetValue(type, out Component? cached))
                return cached as T;

            var component = new T { Entity = this };
            m_ComponentCache[type] = component;
            return component;
        }

        public static Entity? FindByName(string name)
        {
            ulong id = InternalCalls.Entity_FindByName(name);
            return id != 0 ? new Entity(id) : null;
        }

        public static Entity Create(string name)
        {
            ulong id = InternalCalls.Entity_Create(name);
            return new Entity(id);
        }

        public void Destroy() => InternalCalls.Entity_Destroy(ID);

        public bool Equals(Entity? other) => other is not null && ID == other.ID;
        public override bool Equals(object? obj) => obj is Entity other && Equals(other);
        public override int GetHashCode() => (int)ID;

        public static bool operator ==(Entity? a, Entity? b)
        {
            if (a is null) return b is null;
            return a.Equals(b);
        }

        public static bool operator !=(Entity? a, Entity? b) => !(a == b);

        public static implicit operator bool(Entity? entity)
        {
            if (entity is null || entity == Invalid) return false;
            return InternalCalls.Entity_IsValid(entity.ID);
        }
    }
}
