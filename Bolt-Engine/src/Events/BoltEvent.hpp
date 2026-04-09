#pragma once
#include "Core/Base.hpp"
#include <string>
#include <functional>

namespace Bolt {

	enum class EventType {
		None = 0,

		// Window events
		WindowClose,
		WindowResize,
		WindowFocus,
		WindowLostFocus,
		WindowMoved,
		WindowMinimize,

		// Application events
		AppTick,
		AppUpdate,
		AppRender,

		// Input: Keyboard
		KeyPressed,
		KeyReleased,
		KeyTyped,

		// Input: Mouse
		MouseButtonPressed,
		MouseButtonReleased,
		MouseMoved,
		MouseScrolled,

		// Scene lifecycle
		ScenePreStart,
		ScenePostStart,
		ScenePreStop,
		ScenePostStop,

		// Editor
		EditorExitPlayMode,
		SelectionChanged,

		// Assets
		AssetReloaded,

		// File drop
		FileDrop,
	};

	enum EventCategory {
		EventCategoryNone        = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput       = BIT(1),
		EventCategoryKeyboard    = BIT(2),
		EventCategoryMouse       = BIT(3),
		EventCategoryMouseButton = BIT(4),
		EventCategoryScene       = BIT(5),
		EventCategoryEditor      = BIT(6),
	};

#define BT_EVENT_CLASS_TYPE(type)                                           \
	static EventType GetStaticType() { return EventType::type; }            \
	EventType GetEventType() const override { return GetStaticType(); }     \
	const char* GetName() const override { return #type; }

#define BT_EVENT_CLASS_CATEGORY(category)                                   \
	int GetCategoryFlags() const override { return category; }

	class BoltEvent {
	public:
		bool Handled = false;

		virtual ~BoltEvent() = default;
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		bool IsInCategory(EventCategory category) const {
			return (GetCategoryFlags() & category) != 0;
		}
	};

	class EventDispatcher {
	public:
		explicit EventDispatcher(BoltEvent& event)
			: m_Event(event) {}

		template<typename T, typename F>
		bool Dispatch(F&& func) {
			if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.Handled) {
				m_Event.Handled = func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}

	private:
		BoltEvent& m_Event;
	};

	using EventCallbackFn = std::function<void(BoltEvent&)>;

} // namespace Bolt
