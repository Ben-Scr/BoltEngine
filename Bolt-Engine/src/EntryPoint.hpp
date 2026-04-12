#pragma once
#include "Core/Application.hpp"

extern Bolt::Application* CreateApplication();
bool g_ApplicationRunning = true;

namespace Bolt {

	int Main(int argc, char** argv) {
		InitializeCore();
		Application::SetCommandLineArgs(argc, argv);

		try {
			Application* app = Bolt::CreateApplication();
			BT_CORE_ASSERT(app, "Client app is null!");

			app->Run();
			delete app;
			return 0;
		}
		catch (const std::exception& ex) {
			BT_CORE_ERROR("Unhandled exception at app boundary: {}", ex.what());
			return -1;
		}
		catch (...) {
			BT_CORE_ERROR("Unhandled non-std exception at app boundary");
			return -1;
		}

		ShutdownCore();
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
