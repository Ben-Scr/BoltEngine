#pragma once
#include "Scripting/ScriptGlue.hpp"

namespace Bolt {

	/// <summary>
	/// Populates a NativeBindings struct with all engine function pointers.
	/// These are passed to the managed bridge during CoreCLR initialization.
	/// </summary>
	class ScriptBindings {
	public:
		static void PopulateNativeBindings(NativeBindings& bindings);
	};

} // namespace Bolt
