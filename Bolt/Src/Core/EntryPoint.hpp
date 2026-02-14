#pragma once

#ifdef BT_PLATFORM_WINDOWS
extern Bolt::Application* Bolt::CreateApplication();

int main(int argc, char** argv) {
	auto app = Bolt::CreateApplication();
	app->Run();
	delete app;
}
#endif // BT_PLATFORM_WINDOWS
