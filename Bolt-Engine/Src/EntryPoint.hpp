#pragma once
#include "Core/Application.hpp"
#include <iostream>


extern Bolt::Application* CreateApplication();
bool g_ApplicationRunning = true;

namespace Bolt {

	int Main(int argc, char** argv) {
		Application* app = Bolt::CreateApplication();
		BT_CORE_ASSERT(app, "Client app is null!");

		app->Run();
		delete app;
		return 0;
	}
}

#if BT_DIST && BT_PLATFORM_WINDOWS

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	return Bolt::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return Bolt::Main(argc, argv);
}

#endif