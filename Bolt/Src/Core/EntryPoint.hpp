#pragma once

#include "Application.hpp"

#ifdef BT_PLATFORM_WINDOWS
namespace Bolt {
	Application* CreateApplication();
}

int main(int argc, char** argv) {
	auto app = Bolt::CreateApplication();
	app->Run();
	delete app;
}
#endif // BT_PLATFORM_WINDOWS