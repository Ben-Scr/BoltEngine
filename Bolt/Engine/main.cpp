#include <chrono>
#include <filesystem>
#include <iostream>

#include "Core/Application.hpp"
#include "GameSystem.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"


int main() {
   // Bolt::SceneDefinition& def2 = Bolt::SceneManager::RegisterScene("Static Scene");
  //  Bolt::SceneDefinition& def = Bolt::SceneManager::RegisterScene("Test");
   // def.AddSystem<Bolt::GameSystem>();

    Bolt::Application app{};
    app.Run();
    return 0;
}