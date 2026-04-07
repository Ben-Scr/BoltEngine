using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace Bolt.Hosting
{
    /// <summary>
    /// Resolves Bolt-ScriptCore from the default context to prevent duplicate type loading.
    /// </summary>
    internal class ScriptAssemblyLoadContext : AssemblyLoadContext
    {
        private readonly Assembly _coreAssembly;

        public ScriptAssemblyLoadContext(Assembly coreAssembly)
            : base("BoltUserScripts", isCollectible: true)
        {
            _coreAssembly = coreAssembly;
        }

        protected override Assembly? Load(AssemblyName name)
        {
            if (name.Name == _coreAssembly.GetName().Name)
                return _coreAssembly;
            return null;
        }
    }

    /// <summary>
    /// Manages script instances on the managed side. All public methods are
    /// [UnmanagedCallersOnly] and called from C++ via function pointers.
    /// </summary>
    internal static class ScriptInstanceManager
    {
        private static readonly Dictionary<int, ScriptInstanceData> s_Instances = new();
        private static int s_NextHandle = 1;

        private static Assembly? s_CoreAssembly;
        private static Assembly? s_UserAssembly;
        private static AssemblyLoadContext? s_UserLoadContext;
        private static readonly Dictionary<string, ScriptClassInfo?> s_ClassCache = new();

        private struct ScriptInstanceData
        {
            public BoltScript Instance;
            public MethodInfo? StartMethod;
            public MethodInfo? UpdateMethod;
            public MethodInfo? OnDestroyMethod;
            public bool HasStarted;
        }

        private class ScriptClassInfo
        {
            public Type Type = null!;
            public MethodInfo? StartMethod;
            public MethodInfo? UpdateMethod;
            public MethodInfo? OnDestroyMethod;
        }

        internal static void SetCoreAssembly(Assembly assembly)
        {
            s_CoreAssembly = assembly;
        }

        [UnmanagedCallersOnly]
        public static unsafe int CreateScriptInstance(byte* classNamePtr, ulong entityID)
        {
            try
            {
                string className = Marshal.PtrToStringUTF8((IntPtr)classNamePtr)!;
                var classInfo = GetOrCacheClass(className);
                if (classInfo == null) return 0;

                var instance = (BoltScript)Activator.CreateInstance(classInfo.Type)!;
                instance._SetEntityID(entityID);

                int handle = s_NextHandle++;
                s_Instances[handle] = new ScriptInstanceData
                {
                    Instance = instance,
                    StartMethod = classInfo.StartMethod,
                    UpdateMethod = classInfo.UpdateMethod,
                    OnDestroyMethod = classInfo.OnDestroyMethod,
                    HasStarted = false
                };

                return handle;
            }
            catch (Exception ex)
            {
                Log.Error($"CreateScriptInstance failed: {ex}");
                return 0;
            }
        }

        [UnmanagedCallersOnly]
        public static void DestroyScriptInstance(int handle) => s_Instances.Remove(handle);

        [UnmanagedCallersOnly]
        public static void InvokeStart(int handle)
        {
            if (!s_Instances.TryGetValue(handle, out var data) || data.HasStarted) return;

            try
            {
                data.StartMethod?.Invoke(data.Instance, null);
                data.HasStarted = true;
                s_Instances[handle] = data;
            }
            catch (TargetInvocationException ex) { Log.Error($"Exception in Start(): {ex.InnerException}"); }
        }

        [UnmanagedCallersOnly]
        public static void InvokeUpdate(int handle)
        {
            if (!s_Instances.TryGetValue(handle, out var data)) return;
            try { data.UpdateMethod?.Invoke(data.Instance, null); }
            catch (TargetInvocationException ex) { Log.Error($"Exception in Update(): {ex.InnerException}"); }
        }

        [UnmanagedCallersOnly]
        public static void InvokeOnDestroy(int handle)
        {
            if (!s_Instances.TryGetValue(handle, out var data)) return;
            try { data.OnDestroyMethod?.Invoke(data.Instance, null); }
            catch (TargetInvocationException ex) { Log.Error($"Exception in OnDestroy(): {ex.InnerException}"); }
        }

        [UnmanagedCallersOnly]
        public static unsafe int ClassExists(byte* classNamePtr)
        {
            string className = Marshal.PtrToStringUTF8((IntPtr)classNamePtr)!;
            return GetOrCacheClass(className) != null ? 1 : 0;
        }

        [UnmanagedCallersOnly]
        public static unsafe int LoadUserAssembly(byte* pathPtr)
        {
            try
            {
                string path = Marshal.PtrToStringUTF8((IntPtr)pathPtr)!;

                if (s_UserLoadContext != null)
                {
                    s_UserLoadContext.Unload();
                    s_UserLoadContext = null;
                    s_UserAssembly = null;
                }

                s_ClassCache.Clear();
                s_UserLoadContext = new ScriptAssemblyLoadContext(s_CoreAssembly!);

                // Load from bytes to avoid file-locking the DLL on disk
                string fullPath = System.IO.Path.GetFullPath(path);
                byte[] assemblyBytes = System.IO.File.ReadAllBytes(fullPath);
                s_UserAssembly = s_UserLoadContext.LoadFromStream(
                    new System.IO.MemoryStream(assemblyBytes));

                Log.Info($"User assembly loaded: {path}");
                return 1;
            }
            catch (Exception ex)
            {
                Log.Error($"Failed to load user assembly: {ex.Message}");
                return 0;
            }
        }

        [UnmanagedCallersOnly]
        public static void UnloadUserAssembly()
        {
            s_Instances.Clear();
            s_ClassCache.Clear();

            if (s_UserLoadContext != null)
            {
                s_UserLoadContext.Unload();
                s_UserLoadContext = null;
                s_UserAssembly = null;
            }
        }

        private static ScriptClassInfo? GetOrCacheClass(string className)
        {
            if (s_ClassCache.TryGetValue(className, out var cached))
                return cached;

            Type? type = FindType(className);
            if (type == null || !type.IsSubclassOf(typeof(BoltScript)))
            {
                s_ClassCache[className] = null;
                return null;
            }

            var info = new ScriptClassInfo
            {
                Type = type,
                StartMethod = type.GetMethod("Start", BindingFlags.Public | BindingFlags.Instance, Type.EmptyTypes),
                UpdateMethod = type.GetMethod("Update", BindingFlags.Public | BindingFlags.Instance, Type.EmptyTypes),
                OnDestroyMethod = type.GetMethod("OnDestroy", BindingFlags.Public | BindingFlags.Instance, Type.EmptyTypes),
            };

            s_ClassCache[className] = info;
            return info;
        }

        private static Type? FindType(string className)
        {
            if (s_UserAssembly != null)
            {
                var type = s_UserAssembly.GetType($"Bolt.{className}")
                        ?? s_UserAssembly.GetType(className);
                if (type != null) return type;
            }

            if (s_CoreAssembly != null)
            {
                var type = s_CoreAssembly.GetType($"Bolt.{className}")
                        ?? s_CoreAssembly.GetType(className);
                if (type != null) return type;
            }

            return null;
        }
    }
}
