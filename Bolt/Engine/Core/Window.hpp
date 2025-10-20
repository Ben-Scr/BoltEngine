#pragma once
#include "../Core/Input.hpp"
#include "../Collections/Vec2.hpp"
#include "../Collections/Color.hpp"
#include "../Collections/Viewport.hpp"

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
		float GetAspect() const { return s_MainViewport.GetAspect(); }
		Vec2Int GetSize() const { return s_MainViewport.GetSize(); }

		static void SetVsync(bool enabled) { glfwSwapInterval(enabled ? 1 : 0); s_IsVsync = enabled; };
		static bool IsVsync() { return s_IsVsync; }
		void SetWindowResizeable(bool enabled) { glfwWindowHint(GLFW_RESIZABLE, enabled ? GLFW_TRUE : GLFW_FALSE); }
		void SetWindowMoveable(bool enabled) { glfwWindowHint(GLFW_DECORATED, enabled ? GLFW_TRUE : GLFW_FALSE); }



		bool IsMaximized() const;
		bool IsMinimized() const;

		void MaximizeWindow(bool reset = false);
		void RestoreWindow();
		void MinimizeWindow();

		void CenterWindow();
		void FocusWindow();

		void SetTitle(const std::string& title) { glfwSetWindowTitle(m_Window,title.c_str()); }
		std::string GetTitle() const { return std::string(glfwGetWindowTitle(m_Window)); }
		int GetWidth()  const { return s_MainViewport.Width; }
		int GetHeight() const { return s_MainViewport.Height; }

		void Destroy();
		static Window& Main() { return *s_ActiveWindow; }
		static Viewport GetMainViewport() { return s_MainViewport; };

	private:
		void InitWindow();
		void UpdateViewport();
		void UpdateWindowSize();
		void SwapBuffers() const { glfwSwapBuffers(m_Window); }
		void ResetWindowResizedFlag() { m_WindowResized = false; }

		bool IsWindowResized() const { return m_WindowResized; }
		bool ShouldClose() const { return glfwWindowShouldClose(m_Window); }
		Vec2Int GetScreenCenter() const;

		static void RefreshCallback(GLFWwindow* window);
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
		static Viewport s_MainViewport;
		std::string m_Title;
		Color m_BackgroundColor;
		bool m_Resizeable;
		bool m_Moveable;
		bool m_Fullscreen;
		static bool s_IsVsync;

		friend class Application;
	};
}