#pragma once
#include "Core/Export.hpp"
#include "Scene/EntityHandle.hpp"
#include <string>

namespace Bolt {
	class BOLT_API ScriptInstance {
	public:
		ScriptInstance() = default;
		explicit ScriptInstance(const std::string& className)
			: m_ClassName(className) {}

		// ── Metadata ─────────────────────────────
		const std::string& GetClassName() const { return m_ClassName; }
		void SetClassName(const std::string& name) { m_ClassName = name; }

		// ── Entity binding ───────────────────────
		void Bind(EntityHandle entity) { m_Entity = entity; m_IsBound = true; }
		void Unbind() { m_Entity = entt::null; m_IsBound = false; m_HasStarted = false; }
		bool IsBound() const { return m_IsBound; }
		EntityHandle GetEntity() const { return m_Entity; }

		// ── Lifecycle flags ──────────────────────
		bool HasStarted() const { return m_HasStarted; }
		void MarkStarted() { m_HasStarted = true; }

		// ── Future runtime handle ────────────────
		// When Mono/CoreCLR is integrated, this will hold the GCHandle
		// to the managed object so the engine can invoke methods on it.
		void* GetManagedHandle() const { return m_ManagedHandle; }
		void SetManagedHandle(void* handle) { m_ManagedHandle = handle; }

	private:
		std::string m_ClassName;
		EntityHandle m_Entity = entt::null;
		bool m_IsBound = false;
		bool m_HasStarted = false;
		void* m_ManagedHandle = nullptr;
	};

}