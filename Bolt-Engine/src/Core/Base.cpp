#include "pch.hpp"
#include "Base.hpp"

#include "Core/Log.hpp"
//#include "Core/Memory.h"

//#include "Hazel/Renderer/RendererAPI.h"

namespace Bolt {

	void InitializeCore()
	{
		//Platform::Init();
		//Allocator::Init();
		//Log::Init();

//#ifdef BT_HEADLESS
//		RendererAPI::SetAPI(RendererAPIType::None);
//#endif

		BT_CORE_TRACE_TAG("Core", "Hazel Engine {}", BT_VERSION);
		BT_CORE_TRACE_TAG("Core", "Initializing...");
	}

	void ShutdownCore()
	{
		BT_CORE_TRACE_TAG("Core", "Shutting down...");

		//Log::Shutdown();
	}

}