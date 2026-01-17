#pragma once
#include "../Core/Input.hpp"
#include "../Collections/Vec2.hpp"
#include "../Collections/Color.hpp"
#include "../Collections/Viewport.hpp"

#include <GLFW/glfw3native.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

namespace Bolt {
	class Texture2D;

	struct GLFWWindowProperties {
		int Width{ 800 }, Height{ 800 };
		std::string Title{ "GLFW Window" };
		bool Resizeable{ true };
		bool Moveable{ true };
		bool Fullscreen{ false };
		Color BackgroundColor;

		GLFWWindowProperties() = default;

		GLFWWindowProperties(int width, int height, const std::string& title)
			: Width{ width }, Height{ height }, Title{ title }
		{
		}
		GLFWWindowProperties(int width, int height, const std::string& title, bool resizeable, bool moveable, bool fullscreen)
			: Width{ width }, Height{ height }, Title{ title }, Resizeable{ resizeable }, Moveable{ moveable }, Fullscreen{ fullscreen }
		{
		}
	};

	class Window {
	public:
		Window(int width, int height, const std::string& title);
		Window(const GLFWWindowProperties& props);

		GLFWwindow* GLFWWindow() const { return m_Window; }
		float GetAspect() const { return s_MainViewport.GetAspect(); }
		Vec2Int GetSize() const { return s_MainViewport.GetSize(); }
		Vec2Int GetPosition() const {
			Vec2Int pos;
			glfwGetWindowPos(m_Window, &pos.x, &pos.y);
			return pos;
		}

		// Info: Initializes the GLFW library
		static void Initialize();

		// Info: Terminates the GLFW library
		static void Shutdown();

		static void SetVsync(bool enabled) { glfwSwapInterval(enabled); s_IsVsync = enabled; };
		static bool IsVsync() { return s_IsVsync; }

		void SetWindowResizeable(bool enabled) { glfwWindowHint(GLFW_RESIZABLE, enabled ? GLFW_TRUE : GLFW_FALSE); }
		void SetWindowMoveable(bool enabled) { glfwWindowHint(GLFW_DECORATED, enabled ? GLFW_TRUE : GLFW_FALSE); }
		void SetPosition(Vec2Int pos) { glfwSetWindowPos(m_Window, pos.x, pos.y); }
		void SetSize(Vec2Int size) { glfwSetWindowSize(m_Window, size.x, size.y); }
		void SetCursorPosition(Vec2 position) { glfwSetCursorPos(m_Window, (double)position.x, (double)position.y); }

		Vec2 GetCursorPosition() const;

		void SetCursorLocked(bool enabled) {
			glfwSetInputMode(m_Window, GLFW_CURSOR, enabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		}

		void SetCursorHidden(bool enabled) {
			glfwSetInputMode(m_Window, GLFW_CURSOR, enabled ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
		}


		void SetCursorImage(const Texture2D* tex2D);
		void SetWindowIcon(const Texture2D* tex2D);

		bool IsMaximized() const;
		bool IsMinimized() const;
		bool IsFullScreen() const;

		// Info: If reset equals true the window will automatically be restored if it's already maximized
		void MaximizeWindow(bool reset = false);
		void MinimizeWindow();
		void RestoreWindow();

		void CenterWindow();
		void FocusWindow();

		// Info: If enabled equals true the window will become fullscreen else windowed
		void SetFullScreen(bool enabled);
		void SetTitle(const std::string& title) { glfwSetWindowTitle(m_Window, title.c_str()); }

		std::string GetTitle() const { return std::string(glfwGetWindowTitle(m_Window)); }
		int GetWidth()  const { return s_MainViewport.Width; }
		int GetHeight() const { return s_MainViewport.Height; }
		GLFWmonitor* GetWindowMonitor() const;
		static GLFWmonitor* GetMainMonitor();
		const GLFWvidmode* GetVideomode() const { return k_Videomode; }

		// Info: Destroys the window
		void Destroy();

		static Window* Main() { return s_ActiveWindow; }
		static Viewport GetMainViewport() { return s_MainViewport; };

	private:
		void InitWindow(const GLFWWindowProperties& props);
		void UpdateViewport();
		void UpdateWindowSize();
		void SwapBuffers() const { glfwSwapBuffers(m_Window); }

		bool ShouldClose() const { return glfwWindowShouldClose(m_Window); }
		Vec2Int GetScreenCenter() const;
		Vec2 GetWindowCenter() const;

		static void FocusCallback(GLFWwindow* window, int focused);
		static void RefreshCallback(GLFWwindow* window);
		static void SetKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void SetMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void SetCursorPositionCallback(GLFWwindow* window, double xPos, double yPos);
		static void SetScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
		static void SetWindowResizedCallback(GLFWwindow* window, int width, int height);

		static Window* s_ActiveWindow;

		GLFWwindow* m_Window = nullptr;
		GLFWcursor* m_Cursor = nullptr;
		Vec2Int m_RestoreSize;
		Vec2Int m_RestorePos;

		static const GLFWvidmode* k_Videomode;

		Color m_BackgroundColor;

		static Viewport s_MainViewport;
		static bool s_IsVsync;
		static bool s_IsInitialized;

		friend class Application;
	};
}