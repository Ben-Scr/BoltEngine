#include <Scripting/NativeScript.hpp>

extern "C" __declspec(dllexport)
void BoltInitialize(void* engineAPI) {
	Bolt::g_EngineAPI = static_cast<Bolt::NativeEngineAPI*>(engineAPI);
}

extern "C" __declspec(dllexport)
Bolt::NativeScript* BoltCreateScript(const char* className) {
	return Bolt::NativeScriptRegistry::Create(className);
}

extern "C" __declspec(dllexport)
void BoltDestroyScript(Bolt::NativeScript* script) {
	delete script;
}
