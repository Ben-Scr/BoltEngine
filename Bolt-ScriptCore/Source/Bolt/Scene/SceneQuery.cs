using System;
using System.Collections;
using System.Collections.Generic;

namespace Bolt
{
    public class Scene
    {
        public string Name { get; internal set; } = "";
        public bool IsLoaded => false;

        private const int MaxQueryResults = 4096;

        public ComponentQuery<T1> Query<T1>()
            where T1 : Component, new()
            => new ComponentQuery<T1>();

        public ComponentQuery<T1, T2> Query<T1, T2>()
            where T1 : Component, new()
            where T2 : Component, new()
            => new ComponentQuery<T1, T2>();

        public ComponentQuery<T1, T2, T3> Query<T1, T2, T3>()
            where T1 : Component, new()
            where T2 : Component, new()
            where T3 : Component, new()
            => new ComponentQuery<T1, T2, T3>();

        public ComponentQuery<T1, T2, T3, T4> Query<T1, T2, T3, T4>()
            where T1 : Component, new()
            where T2 : Component, new()
            where T3 : Component, new()
            where T4 : Component, new()
            => new ComponentQuery<T1, T2, T3, T4>();

        /// <summary>Returns the number of entities in this scene.</summary>
        public int EntityCount => InternalCalls.Scene_GetEntityCount();

        // ── Internal helpers (static — no instance state needed) ────

        internal static string? GetNativeName<T>() where T : Component, new()
            => Entity.GetNativeComponentName<T>();

        internal static ulong[] ExecuteQuery(string queryString)
        {
            Span<ulong> buffer = stackalloc ulong[MaxQueryResults];
            int count = InternalCalls.Scene_QueryEntities(queryString, buffer);
            if (count <= 0) return Array.Empty<ulong>();

            int resultCount = Math.Min(count, MaxQueryResults);
            ulong[] result = new ulong[resultCount];
            buffer.Slice(0, resultCount).CopyTo(result);
            return result;
        }

        internal static string BuildQueryString(params string?[] names)
        {
            for (int i = 0; i < names.Length; i++)
                if (names[i] == null) return "";
            return string.Join('|', names!);
        }
    }

    // ── 1 Component ─────────────────────────────────────────────────────

    public struct ComponentQuery<T1> : IEnumerable<T1>
        where T1 : Component, new()
    {
        public EntityComponentQuery<T1> WithEntity() => new EntityComponentQuery<T1>();

        public IEnumerator<T1> GetEnumerator()
        {
            string query = Scene.BuildQueryString(Scene.GetNativeName<T1>());
            if (string.IsNullOrEmpty(query)) yield break;

            foreach (ulong id in Scene.ExecuteQuery(query))
            {
                var entity = new Entity(id);
                yield return entity.GetComponent<T1>()!;
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }

    public struct EntityComponentQuery<T1> : IEnumerable<(Entity Entity, T1 C1)>
        where T1 : Component, new()
    {
        public IEnumerator<(Entity, T1)> GetEnumerator()
        {
            string query = Scene.BuildQueryString(Scene.GetNativeName<T1>());
            if (string.IsNullOrEmpty(query)) yield break;

            foreach (ulong id in Scene.ExecuteQuery(query))
            {
                var entity = new Entity(id);
                yield return (entity, entity.GetComponent<T1>()!);
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }

    // ── 2 Components ────────────────────────────────────────────────────

    public struct ComponentQuery<T1, T2> : IEnumerable<(T1 C1, T2 C2)>
        where T1 : Component, new()
        where T2 : Component, new()
    {
        public EntityComponentQuery<T1, T2> WithEntity() => new EntityComponentQuery<T1, T2>();

        public IEnumerator<(T1, T2)> GetEnumerator()
        {
            string query = Scene.BuildQueryString(Scene.GetNativeName<T1>(), Scene.GetNativeName<T2>());
            if (string.IsNullOrEmpty(query)) yield break;

            foreach (ulong id in Scene.ExecuteQuery(query))
            {
                var entity = new Entity(id);
                yield return (entity.GetComponent<T1>()!, entity.GetComponent<T2>()!);
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }

    public struct EntityComponentQuery<T1, T2> : IEnumerable<(Entity Entity, T1 C1, T2 C2)>
        where T1 : Component, new()
        where T2 : Component, new()
    {
        public IEnumerator<(Entity, T1, T2)> GetEnumerator()
        {
            string query = Scene.BuildQueryString(Scene.GetNativeName<T1>(), Scene.GetNativeName<T2>());
            if (string.IsNullOrEmpty(query)) yield break;

            foreach (ulong id in Scene.ExecuteQuery(query))
            {
                var entity = new Entity(id);
                yield return (entity, entity.GetComponent<T1>()!, entity.GetComponent<T2>()!);
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }

    // ── 3 Components ────────────────────────────────────────────────────

    public struct ComponentQuery<T1, T2, T3> : IEnumerable<(T1 C1, T2 C2, T3 C3)>
        where T1 : Component, new()
        where T2 : Component, new()
        where T3 : Component, new()
    {
        public EntityComponentQuery<T1, T2, T3> WithEntity() => new EntityComponentQuery<T1, T2, T3>();

        public IEnumerator<(T1, T2, T3)> GetEnumerator()
        {
            string query = Scene.BuildQueryString(Scene.GetNativeName<T1>(), Scene.GetNativeName<T2>(), Scene.GetNativeName<T3>());
            if (string.IsNullOrEmpty(query)) yield break;

            foreach (ulong id in Scene.ExecuteQuery(query))
            {
                var entity = new Entity(id);
                yield return (entity.GetComponent<T1>()!, entity.GetComponent<T2>()!, entity.GetComponent<T3>()!);
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }

    public struct EntityComponentQuery<T1, T2, T3> : IEnumerable<(Entity Entity, T1 C1, T2 C2, T3 C3)>
        where T1 : Component, new()
        where T2 : Component, new()
        where T3 : Component, new()
    {
        public IEnumerator<(Entity, T1, T2, T3)> GetEnumerator()
        {
            string query = Scene.BuildQueryString(Scene.GetNativeName<T1>(), Scene.GetNativeName<T2>(), Scene.GetNativeName<T3>());
            if (string.IsNullOrEmpty(query)) yield break;

            foreach (ulong id in Scene.ExecuteQuery(query))
            {
                var entity = new Entity(id);
                yield return (entity, entity.GetComponent<T1>()!, entity.GetComponent<T2>()!, entity.GetComponent<T3>()!);
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }

    // ── 4 Components ────────────────────────────────────────────────────

    public struct ComponentQuery<T1, T2, T3, T4> : IEnumerable<(T1 C1, T2 C2, T3 C3, T4 C4)>
        where T1 : Component, new()
        where T2 : Component, new()
        where T3 : Component, new()
        where T4 : Component, new()
    {
        public EntityComponentQuery<T1, T2, T3, T4> WithEntity() => new EntityComponentQuery<T1, T2, T3, T4>();

        public IEnumerator<(T1, T2, T3, T4)> GetEnumerator()
        {
            string query = Scene.BuildQueryString(Scene.GetNativeName<T1>(), Scene.GetNativeName<T2>(), Scene.GetNativeName<T3>(), Scene.GetNativeName<T4>());
            if (string.IsNullOrEmpty(query)) yield break;

            foreach (ulong id in Scene.ExecuteQuery(query))
            {
                var entity = new Entity(id);
                yield return (entity.GetComponent<T1>()!, entity.GetComponent<T2>()!, entity.GetComponent<T3>()!, entity.GetComponent<T4>()!);
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }

    public struct EntityComponentQuery<T1, T2, T3, T4> : IEnumerable<(Entity Entity, T1 C1, T2 C2, T3 C3, T4 C4)>
        where T1 : Component, new()
        where T2 : Component, new()
        where T3 : Component, new()
        where T4 : Component, new()
    {
        public IEnumerator<(Entity, T1, T2, T3, T4)> GetEnumerator()
        {
            string query = Scene.BuildQueryString(Scene.GetNativeName<T1>(), Scene.GetNativeName<T2>(), Scene.GetNativeName<T3>(), Scene.GetNativeName<T4>());
            if (string.IsNullOrEmpty(query)) yield break;

            foreach (ulong id in Scene.ExecuteQuery(query))
            {
                var entity = new Entity(id);
                yield return (entity, entity.GetComponent<T1>()!, entity.GetComponent<T2>()!, entity.GetComponent<T3>()!, entity.GetComponent<T4>()!);
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }
}
