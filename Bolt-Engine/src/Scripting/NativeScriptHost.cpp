#include "pch.hpp"
#include "Scripting/NativeScriptHost.hpp"
#include "Scripting/NativeScript.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Core/Log.hpp"
#include "Core/Application.hpp"
#include "Core/Time.hpp"
#include "Scene/Scene.hpp"
#include "Components/General/Transform2DComponent.hpp"

#ifdef BT_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace Bolt {

	// Engine API implementations — these run IN the engine process,
	// called through function pointers from the DLL.

	static void API_LogInfo(const char* msg) {
		Log::PrintMessageTag(Log::Type::Client, Log::Level::Info, "NativeScript", msg);
	}
	static void API_LogWarn(const char* msg) {
		Log::PrintMessageTag(Log::Type::Client, Log::Level::Warn, "NativeScript", msg);
	}
	static void API_LogError(const char* msg) {
		Log::PrintMessageTag(Log::Type::Client, Log::Level::Error, "NativeScript", msg);
	}
	static float API_GetDeltaTime() {
		auto* app = Application::GetInstance();
		return app ? app->GetTime().GetDeltaTime() : 0.0f;
	}
	static void API_GetPosition(uint32_t entityID, float* x, float* y) {
		Scene* scene = ScriptEngine::GetScene();
		if (!scene) { *x = 0; *y = 0; return; }
		auto handle = static_cast<EntityHandle>(entityID);
		if (!scene->HasComponent<Transform2DComponent>(handle)) { *x = 0; *y = 0; return; }
		auto& t = scene->GetComponent<Transform2DComponent>(handle);
		*x = t.Position.x; *y = t.Position.y;
	}
	static void API_SetPosition(uint32_t entityID, float x, float y) {
		Scene* scene = ScriptEngine::GetScene();
		if (!scene) return;
		auto handle = static_cast<EntityHandle>(entityID);
		if (!scene->HasComponent<Transform2DComponent>(handle)) return;
		scene->GetComponent<Transform2DComponent>(handle).Position = { x, y };
	}
	static float API_GetRotation(uint32_t entityID) {
		Scene* scene = ScriptEngine::GetScene();
		if (!scene) return 0.0f;
		auto handle = static_cast<EntityHandle>(entityID);
		if (!scene->HasComponent<Transform2DComponent>(handle)) return 0.0f;
		return scene->GetComponent<Transform2DComponent>(handle).Rotation;
	}
	static void API_SetRotation(uint32_t entityID, float rot) {
		Scene* scene = ScriptEngine::GetScene();
		if (!scene) return;
		auto handle = static_cast<EntityHandle>(entityID);
		if (!scene->HasComponent<Transform2DComponent>(handle)) return;
		scene->GetComponent<Transform2DComponent>(handle).Rotation = rot;
	}

	static NativeEngineAPI s_EngineAPI = {
		API_LogInfo, API_LogWarn, API_LogError,
		API_GetDeltaTime,
		API_GetPosition, API_SetPosition,
		API_GetRotation, API_SetRotation
	};

	bool NativeScriptHost::LoadDLL(const std::string& dllPath)
	{
		if (m_DllHandle) UnloadDLL();
		m_DllPath = dllPath;

#ifdef BT_PLATFORM_WINDOWS
		m_DllHandle = static_cast<void*>(LoadLibraryA(dllPath.c_str()));
		if (!m_DllHandle)
		{
			BT_CORE_ERROR_TAG("NativeScriptHost", "Failed to load DLL: {}", dllPath);
			return false;
		}

		m_CreateFn = reinterpret_cast<CreateFn>(
			GetProcAddress(static_cast<HMODULE>(m_DllHandle), "BoltCreateScript"));
		m_DestroyFn = reinterpret_cast<DestroyFn>(
			GetProcAddress(static_cast<HMODULE>(m_DllHandle), "BoltDestroyScript"));

		if (!m_CreateFn || !m_DestroyFn)
		{
			BT_CORE_ERROR_TAG("NativeScriptHost",
				"DLL missing BoltCreateScript/BoltDestroyScript: {}", dllPath);
			FreeLibrary(static_cast<HMODULE>(m_DllHandle));
			m_DllHandle = nullptr;
			return false;
		}

		// Pass engine API to the DLL so scripts can call engine functions
		auto initFn = reinterpret_cast<InitFn>(
			GetProcAddress(static_cast<HMODULE>(m_DllHandle), "BoltInitialize"));
		if (initFn)
			initFn(&s_EngineAPI);
#endif

		BT_CORE_INFO_TAG("NativeScriptHost", "Loaded native script DLL: {}", dllPath);
		return true;
	}

	void NativeScriptHost::UnloadDLL()
	{
		DestroyAllInstances();

#ifdef BT_PLATFORM_WINDOWS
		if (m_DllHandle)
		{
			FreeLibrary(static_cast<HMODULE>(m_DllHandle));
		}
#endif

		m_DllHandle = nullptr;
		m_CreateFn = nullptr;
		m_DestroyFn = nullptr;
	}

	bool NativeScriptHost::Reload()
	{
		if (m_DllPath.empty()) return false;
		std::string path = m_DllPath;
		UnloadDLL();
		return LoadDLL(path);
	}

	NativeScript* NativeScriptHost::CreateInstance(
		const std::string& className, EntityHandle entity, Scene* scene)
	{
		if (!m_CreateFn) return nullptr;

		NativeScript* script = m_CreateFn(className.c_str());
		if (!script) return nullptr;

		script->m_EntityID = static_cast<uint32_t>(entity);
		m_LiveInstances.push_back(script);
		return script;
	}

	void NativeScriptHost::DestroyInstance(NativeScript* script)
	{
		if (!script) return;
		script->OnDestroy();

		auto it = std::find(m_LiveInstances.begin(), m_LiveInstances.end(), script);
		if (it != m_LiveInstances.end())
			m_LiveInstances.erase(it);

		if (m_DestroyFn)
			m_DestroyFn(script);
	}

	void NativeScriptHost::DestroyAllInstances()
	{
		for (auto* script : m_LiveInstances)
		{
			script->OnDestroy();
			if (m_DestroyFn) m_DestroyFn(script);
		}
		m_LiveInstances.clear();
	}

	bool NativeScriptHost::HasClass(const std::string& className)
	{
		if (!m_CreateFn) return false;
		NativeScript* test = m_CreateFn(className.c_str());
		if (!test) return false;
		if (m_DestroyFn) m_DestroyFn(test);
		return true;
	}

} // namespace Bolt
