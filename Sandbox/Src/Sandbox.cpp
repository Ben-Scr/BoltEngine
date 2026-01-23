#include <Bolt.hpp>
#include "GameSystem.hpp"
#include "Systems/ParticleUpdateSystem.hpp"

int main() {
	Bolt::SceneDefinition& def = Bolt::SceneManager::RegisterScene("Game");
	def.AddSystem<GameSystem>();
	def.AddSystem<Bolt::ParticleUpdateSystem>();

	Bolt::Application::SetRunInBackground(true);
	Bolt::Application::SetForceSingleInstance(true);
	Bolt::Application app;
	app.Run();
	return 0;
}