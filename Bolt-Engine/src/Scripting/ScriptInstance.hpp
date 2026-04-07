#pragma once
#include "Core/Export.hpp"
#include "Scene/EntityHandle.hpp"
#include <string>
#include <cstdint>

namespace Bolt {

	class BOLT_API ScriptInstance {
	public:
		ScriptInstance() = default;
		explicit ScriptInstance(const std::string& className)
			: m_ClassName(className) {}

		const std::string& GetClassName() const { return m_ClassName; }
		void SetClassName(const std::string& name) { m_ClassName = name; }

		void Bind(EntityHandle entity) { m_Entity = entity; m_IsBound = true; }
		void Unbind() { m_Entity = entt::null; m_IsBound = false; m_HasStarted = false; m_GCHandle = 0; }
		bool IsBound() const { return m_IsBound; }
		EntityHandle GetEntity() const { return m_Entity; }

		bool HasStarted() const { return m_HasStarted; }
		void MarkStarted() { m_HasStarted = true; }

		uint32_t GetGCHandle() const { return m_GCHandle; }
		void SetGCHandle(uint32_t handle) { m_GCHandle = handle; }
		bool HasManagedInstance() const { return m_GCHandle != 0; }

	private:
		std::string m_ClassName;
		EntityHandle m_Entity = entt::null;
		bool m_IsBound = false;
		bool m_HasStarted = false;
		uint32_t m_GCHandle = 0;
	};

} // namespace Bolt
