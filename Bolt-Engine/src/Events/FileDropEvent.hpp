#pragma once

#include "Events/BoltEvent.hpp"

#include <string>
#include <utility>
#include <vector>

namespace Bolt {

	class FileDropEvent : public BoltEvent {
	public:
		explicit FileDropEvent(std::vector<std::string> paths)
			: m_Paths(std::move(paths)) {
		}

		const std::vector<std::string>& GetPaths() const { return m_Paths; }
		int GetCount() const { return static_cast<int>(m_Paths.size()); }

		BT_EVENT_CLASS_TYPE(FileDrop)
		BT_EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		std::vector<std::string> m_Paths;
	};

} // namespace Bolt
