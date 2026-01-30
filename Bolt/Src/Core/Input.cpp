#include "pch.hpp"
#include "Input.hpp"

namespace Bolt {
    bool  Input::s_CurrentKeyStates[k_KeyCount] = { false };
    bool  Input::s_PreviousKeyStates[k_KeyCount] = { false };
    bool  Input::s_CurrentMouseButtons[k_MouseCount] = { false };
    bool  Input::s_PreviousMouseButtons[k_MouseCount] = { false };
    float Input::s_ScrollValue = 0.f;
    Vec2 Input::s_MousePosition = { 0,0 };
    Vec2 Input::s_MouseDelta = { 0,0 };
	Vec2 Input::s_Axis = { 0,0 };
}