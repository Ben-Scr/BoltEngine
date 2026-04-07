#pragma once
#include <cstdint>
#include <cstring>

namespace Bolt {

	class Scene;

	/// Engine API passed to native script DLLs at load time.
	/// Scripts call these instead of linking against the engine.
	struct NativeEngineAPI {
		void (*LogInfo)(const char* msg);
		void (*LogWarn)(const char* msg);
		void (*LogError)(const char* msg);
		float (*GetDeltaTime)();
		void (*GetPosition)(uint32_t entity, float* x, float* y);
		void (*SetPosition)(uint32_t entity, float x, float y);
		float (*GetRotation)(uint32_t entity);
		void (*SetRotation)(uint32_t entity, float rot);
	};

	/// Global engine API pointer — set by the engine when the DLL is loaded.
	/// Scripts access it via the BT_LOG_INFO etc. macros below.
	inline NativeEngineAPI* g_EngineAPI = nullptr;

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

	/// Auto-registration registry
	class NativeScriptRegistry {
	public:
		using Factory = NativeScript* (*)();

		struct Entry { const char* name; Factory factory; Entry* next; };
		inline static Entry* s_Head = nullptr;

		static void Register(const char* name, Factory factory) {
			static Entry entries[256];
			static int count = 0;
			if (count < 256) {
				entries[count] = { name, factory, s_Head };
				s_Head = &entries[count++];
			}
		}

		static NativeScript* Create(const char* name) {
			for (auto* e = s_Head; e; e = e->next)
				if (strcmp(name, e->name) == 0) return e->factory();
			return nullptr;
		}

		static bool Has(const char* name) {
			for (auto* e = s_Head; e; e = e->next)
				if (strcmp(name, e->name) == 0) return true;
			return false;
		}
	};

} // namespace Bolt

// Logging macros for native scripts — route through engine API
#define BT_NATIVE_LOG_INFO(msg)  do { if (Bolt::g_EngineAPI && Bolt::g_EngineAPI->LogInfo) Bolt::g_EngineAPI->LogInfo(msg); } while(0)
#define BT_NATIVE_LOG_WARN(msg)  do { if (Bolt::g_EngineAPI && Bolt::g_EngineAPI->LogWarn) Bolt::g_EngineAPI->LogWarn(msg); } while(0)
#define BT_NATIVE_LOG_ERROR(msg) do { if (Bolt::g_EngineAPI && Bolt::g_EngineAPI->LogError) Bolt::g_EngineAPI->LogError(msg); } while(0)

#define REGISTER_SCRIPT(ClassName) \
	static struct ClassName##_AutoReg { \
		ClassName##_AutoReg() { Bolt::NativeScriptRegistry::Register(#ClassName, []() -> Bolt::NativeScript* { return new ClassName(); }); } \
	} s_##ClassName##_autoreg;
