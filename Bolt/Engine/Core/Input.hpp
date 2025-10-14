#pragma once
#include "KeyCodes.hpp"
#include <GLFW/glfw3.h>

namespace Bolt { class Application; }

namespace Bolt {
    class Input {
    public:
        friend class Application;

        // Keyboard
        static bool GetKey(KeyCode k) {
            int i = static_cast<int>(k);
            return (i >= 0 && i < k_KeyCount) ? s_CurrentKeyStates[i] : false;
        }
        static bool GetKeyDown(KeyCode k) {
            int i = static_cast<int>(k);
            return (i >= 0 && i < k_KeyCount)
                && s_CurrentKeyStates[i] && !s_PreviousKeyStates[i];
        }
        static bool GetKeyUp(KeyCode k) {
            int i = static_cast<int>(k);
            return (i >= 0 && i < k_KeyCount)
                && !s_CurrentKeyStates[i] && s_PreviousKeyStates[i];
        }

        // Mouse
        static bool GetMouse(int btn) {
            return (btn >= 0 && btn < k_MouseCount) ? s_CurrentMouseButtons[btn] : false;
        }
        static bool GetMouseDown(int btn) {
            return (btn >= 0 && btn < k_MouseCount)
                && s_CurrentMouseButtons[btn] && !s_PreviousMouseButtons[btn];
        }
        static bool GetMouseUp(int btn) {
            return (btn >= 0 && btn < k_MouseCount)
                && !s_CurrentMouseButtons[btn] && s_PreviousMouseButtons[btn];
        }
        static Vec2 GetAxis() {
            return s_Axis;
        }

        static Vec2 MousePosition() { return s_MousePosition; }
        static Vec2 MouseDelta() { return s_MouseDelta; }
        static float ScrollValue() { return s_ScrollValue; }

    private:
        static void Update() {
            std::copy(std::begin(s_CurrentKeyStates), std::end(s_CurrentKeyStates), std::begin(s_PreviousKeyStates));
            std::copy(std::begin(s_CurrentMouseButtons), std::end(s_CurrentMouseButtons), std::begin(s_PreviousMouseButtons));
            s_ScrollValue = 0.f;
            s_MouseDelta = { 0, 0 };

            bool inputRight = Input::GetKey(KeyCode::D) || Input::GetKey(KeyCode::Right);
            bool inputUp = Input::GetKey(KeyCode::W) || Input::GetKey(KeyCode::Up);
            bool inputDown = Input::GetKey(KeyCode::S) || Input::GetKey(KeyCode::Down);

            bool inputLeft = Input::GetKey(KeyCode::A) || Input::GetKey(KeyCode::Left);

            float x = inputRight ? 1 : (inputLeft ? -1 : 0);
            float y = inputUp ? 1 : (inputDown ? -1 : 0);
            s_Axis = { x, y };
        }


        static void OnKeyDown(int k) { if (k >= 0 && k < k_KeyCount) s_CurrentKeyStates[k] = true; }
        static void OnKeyUp(int k) { if (k >= 0 && k < k_KeyCount) s_CurrentKeyStates[k] = false; }
        static void OnMouseDown(int btn) { if (btn >= 0 && btn < k_MouseCount) s_CurrentMouseButtons[btn] = true; }
        static void OnMouseUp(int btn) { if (btn >= 0 && btn < k_MouseCount) s_CurrentMouseButtons[btn] = false; }
        static void OnScroll(float delta) { s_ScrollValue += delta; }
        static void OnMouseMove(int x, int y) {
            Vec2 pos{ float(x), float(y) };
            s_MouseDelta = pos - s_MousePosition;
            s_MousePosition = pos;
        }

        static const int k_KeyCount = GLFW_KEY_MENU + 1;
        static const int k_MouseCount = 8;

        static bool  s_CurrentKeyStates[k_KeyCount];
        static bool  s_PreviousKeyStates[k_KeyCount];
        static bool  s_CurrentMouseButtons[k_MouseCount];
        static bool  s_PreviousMouseButtons[k_MouseCount];

        static float s_ScrollValue;
        static Vec2 s_MousePosition;
        static Vec2 s_MouseDelta;
        static Vec2 s_Axis;
    };
}