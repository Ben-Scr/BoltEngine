#pragma once
#include "../Core/Input.hpp"
#include "../Collections/Vec2.hpp"
#include "../Collections/Color.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

namespace Bolt {
	struct GLFWWindowProperties {
		int Width{ 800 }, Height{800};
		std::string Title{ "GLFW Window" };
		bool Resizeable{true};
		bool Moveable{true};
		bool Fullscreen{false};
		Color BackgroundColor;
	};

	class Window {
	public:
		Window(int width, int height, const std::string& title);
		Window(const GLFWWindowProperties& windowProps);

		GLFWwindow* GLFWWindow() const { return m_Window; }
		float GetAspect() const { return static_cast<float>(m_Width) / static_cast<float>(m_Height); }
		Vec2Int GetSize() const { return { m_Width, m_Height }; }

		static void SetVsync(bool enabled) { glfwSwapInterval(enabled ? 1 : 0); };
		void SetWindowResizeable(bool enabled) { glfwWindowHint(GLFW_RESIZABLE, enabled ? GLFW_TRUE : GLFW_FALSE); }
		void SetWindowMoveable(bool enabled) { glfwWindowHint(GLFW_DECORATED, enabled ? GLFW_TRUE : GLFW_FALSE); }



		bool IsMaximized() const;
		bool IsMinimized() const;

		void MaximizeWindow(bool reset = false);
		void RestoreWindow();
		void MinimizeWindow();

		void CenterWindow();
		void FocusWindow();

		std::string GetTitle() const { return m_Title; }
		int GetWidth()  const { return m_Width; }
		int GetHeight() const { return m_Height; }

		void Destroy();
		static Window& Main() { return *s_ActiveWindow; }

	private:
		void InitWindow();
		void UpdateViewport();
		void UpdateWindowSize();
		void SwapBuffers() const { glfwSwapBuffers(m_Window); }
		void ResetWindowResizedFlag() { m_WindowResized = false; }

		bool IsWindowResized() const { return m_WindowResized; }
		bool ShouldClose() const { return glfwWindowShouldClose(m_Window); }
		Vec2Int GetScreenCenter() const;

		static void SetKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void SetMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void SetCursorPositionCallback(GLFWwindow* window, double xPos, double yPos);
		static void SetScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
		static void SetWindowResizedCallback(GLFWwindow* window, int width, int height);

		static Window* s_ActiveWindow;

		GLFWwindow* m_Window = nullptr;
		GLFWmonitor* m_Monitor = nullptr;
		const GLFWvidmode* k_Mode = nullptr;

		bool m_WindowResized = false;

		int m_Width, m_Height;
		std::string m_Title;
		Color m_BackgroundColor;
		bool m_Resizeable;
		bool m_Moveable;
		bool m_Fullscreen;

		friend class Application;
	};
}