#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "../Scene/SceneDefinition.hpp"

namespace Bolt {
    class PhysicsSystem;
}

namespace Bolt {
    class Scene;

    class SceneManager {
    public:
        static SceneDefinition& RegisterScene(const std::string& name);
        static Scene& LoadScene(const std::string& name);

        static Scene& LoadSceneAdditive(const std::string& name);
        static Scene& ReloadScene(const std::string name);


        static void UnloadScene(const std::string& name);
        static void UnloadAllScenes(bool includePersistent = false);

        static Scene& GetLoadedScene(const std::string& name);
        static Scene& GetActiveScene();

        static void SetActiveScene(const std::string& name);

        static bool HasSceneDefinition(const std::string& name);
        static bool IsSceneLoaded(const std::string& name);


        static std::vector<std::string> GetRegisteredSceneNames();
        static std::vector<std::string> GetLoadedSceneNames();

    private:
        static void Initialize();
        static void UpdateScenes();
        static void FixedUpdateScenes();
        static void InitializeStartupScenes();

        static std::unordered_map<std::string, std::unique_ptr<SceneDefinition>> s_SceneDefinitions;
        static std::vector<std::unique_ptr<Scene>> s_LoadedScenes;
        static Scene* s_ActiveScene;

        friend class Application;
        friend class PhysicsSystem;
    };
}