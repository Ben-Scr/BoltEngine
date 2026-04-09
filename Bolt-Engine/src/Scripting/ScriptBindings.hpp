#pragma once
#include "Scripting/ScriptGlue.hpp"

namespace Bolt {
	class ScriptBindings {
	public:
		static void PopulateNativeBindings(NativeBindings& bindings);
	};

}
