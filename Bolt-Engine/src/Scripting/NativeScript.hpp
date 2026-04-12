#pragma once

#include "Scripting/NativeEngineAPI.hpp"
#include "Scripting/NativeScriptRegistry.hpp"

#include <cstdint>

namespace Bolt {

	class NativeScript {
	public:
		virtual ~NativeScript() = default;

		virtual void Start() {}
		virtual void Update(float deltaTime) {}
		virtual void OnDestroy() {}

		uint32_t GetEntityID() const { return m_EntityID; }

		// Convenience: position/rotation access via engine API
		void GetPosition(float& x, float& y) const {
			if (g_EngineAPI) g_EngineAPI->GetPosition(m_EntityID, &x, &y);
		}
		void SetPosition(float x, float y) {
			if (g_EngineAPI) g_EngineAPI->SetPosition(m_EntityID, x, y);
		}
		float GetRotation() const {
			return g_EngineAPI ? g_EngineAPI->GetRotation(m_EntityID) : 0.0f;
		}
		void SetRotation(float rot) {
			if (g_EngineAPI) g_EngineAPI->SetRotation(m_EntityID, rot);
		}

	private:
		friend class NativeScriptHost;
		uint32_t m_EntityID = 0;
	};

} // namespace Bolt

#define BT_NATIVE_LOG_INFO(msg)  do { if (Bolt::g_EngineAPI && Bolt::g_EngineAPI->LogInfo) Bolt::g_EngineAPI->LogInfo(msg); } while(0)
#define BT_NATIVE_LOG_WARN(msg)  do { if (Bolt::g_EngineAPI && Bolt::g_EngineAPI->LogWarn) Bolt::g_EngineAPI->LogWarn(msg); } while(0)
#define BT_NATIVE_LOG_ERROR(msg) do { if (Bolt::g_EngineAPI && Bolt::g_EngineAPI->LogError) Bolt::g_EngineAPI->LogError(msg); } while(0)

#define REGISTER_SCRIPT(ClassName) \
	static struct ClassName##_AutoReg { \
		ClassName##_AutoReg() { Bolt::NativeScriptRegistry::Register(#ClassName, []() -> Bolt::NativeScript* { return new ClassName(); }); } \
	} s_##ClassName##_autoreg;
