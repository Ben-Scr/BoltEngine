#pragma once
#include "Core/Application.hpp"
#include <iostream>

#ifdef BT_PLATFORM_WINDOWS
namespace Bolt {
	Application* CreateApplication();
}

int main(int argc, char** argv) {
	Bolt::Application* app = Bolt::CreateApplication();
	try {
		app->Run();
	}
	catch(const std::exception& ex){
		std::cout << ex.what();
	}
	catch (...) {
		std::cout << "Unknown Exception";
	}
	delete app;
}
#endif // BT_PLATFORM_WINDOWS