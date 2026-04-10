using System;
using System.Threading.Tasks;

namespace Bolt
{
    public static class SceneManager
    {
        /// <summary>
        /// Returns the currently active scene.
        /// </summary>
        public static Scene? GetActiveScene()
        {
            string name = InternalCalls.Scene_GetActiveSceneName();
            if (string.IsNullOrEmpty(name)) return null;
            return new Scene { Name = name };
        }

        /// <summary>
        /// Loads a scene by name. If additive is true, the scene is loaded
        /// alongside existing scenes. If false, all non-persistent scenes
        /// are unloaded first.
        /// </summary>
        public static Scene? LoadScene(string name, bool additive = false)
        {
            if (additive)
            {
                if (InternalCalls.Scene_LoadAdditive(name))
                    return new Scene { Name = name };
                return null;
            }

            Log.Warn($"[SceneManager] Non-additive LoadScene('{name}') from scripts is not yet supported.");
            return null;
        }

        /// <summary>
        /// Loads a scene additively (shorthand for LoadScene(name, additive: true)).
        /// </summary>
        public static Scene? LoadSceneAdditive(string name)
        {
            return LoadScene(name, additive: true);
        }

        /// <summary>
        /// Async version of LoadScene.
        /// </summary>
        public static async Task<Scene?> LoadSceneAsync(string name, bool additive = false)
        {
            return await Task.FromResult(LoadScene(name, additive));
        }

        /// <summary>
        /// Unloads a scene by name. Persistent scenes are not unloaded.
        /// </summary>
        public static void UnloadScene(string name)
        {
            InternalCalls.Scene_Unload(name);
        }

        /// <summary>
        /// Async version of UnloadScene.
        /// </summary>
        public static async Task UnloadSceneAsync(string name)
        {
            UnloadScene(name);
            await Task.CompletedTask;
        }

        /// <summary>
        /// Sets the active scene by name. Returns true if successful.
        /// </summary>
        public static bool SetActiveScene(string name)
        {
            return InternalCalls.Scene_SetActive(name);
        }

        /// <summary>
        /// Returns a loaded scene by name, or null if not loaded.
        /// </summary>
        public static Scene? GetLoadedScene(string name)
        {
            int count = InternalCalls.Scene_GetLoadedCount();
            for (int i = 0; i < count; i++)
            {
                string loadedName = InternalCalls.Scene_GetLoadedSceneNameAt(i);
                if (string.Equals(loadedName, name, StringComparison.OrdinalIgnoreCase))
                    return new Scene { Name = loadedName };
            }
            return null;
        }

        /// <summary>
        /// Returns all currently loaded scenes.
        /// </summary>
        public static Scene[] GetLoadedScenes()
        {
            int count = InternalCalls.Scene_GetLoadedCount();
            var scenes = new Scene[count];
            for (int i = 0; i < count; i++)
                scenes[i] = new Scene { Name = InternalCalls.Scene_GetLoadedSceneNameAt(i) };
            return scenes;
        }

        /// <summary>
        /// Returns the number of currently loaded scenes.
        /// </summary>
        public static int LoadedSceneCount => InternalCalls.Scene_GetLoadedCount();

        /// <summary>
        /// Reloads a scene by name.
        /// </summary>
        public static void ReloadScene(string name)
        {
            Log.Warn($"[SceneManager] ReloadScene('{name}') from scripts is not yet supported.");
        }
    }
}
