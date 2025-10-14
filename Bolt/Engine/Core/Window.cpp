#include "../pch.hpp"
#include "Window.hpp"

namespace Bolt {
	Window* Window::s_ActiveWindow = nullptr;
	bool Window::s_IsVsync = true;

	Window::Window(int width, int height, const std::string& title)
		: m_Width{ width }, m_Height{ height }, m_Title{ title } {
		InitWindow();
	}

	Window::Window(const GLFWWindowProperties& windowProps)
		: m_Width{ windowProps.Width }, m_Height{ windowProps.Height }, m_Title{ windowProps.Title }, m_Fullscreen{ windowProps.Fullscreen },
		m_Resizeable{ m_Resizeable }, m_Moveable{ windowProps.Moveable }, m_BackgroundColor{ windowProps.BackgroundColor } {
		InitWindow();
	}

	void Window::InitWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_SAMPLES, 8);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


		glfwWindowHint(GLFW_DECORATED, m_Moveable);
		glfwWindowHint(GLFW_RESIZABLE, m_Resizeable);

		m_Monitor = glfwGetPrimaryMonitor();
		k_Mode = glfwGetVideoMode(m_Monitor);


		if (m_Fullscreen) {
			m_Window = glfwCreateWindow(k_Mode->width, k_Mode->height, m_Title.c_str(), nullptr, nullptr);
			MaximizeWindow();
		}
		else {
			m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
			CenterWindow();
		}

		assert(m_Window && "Failed to create window!");

		glfwMakeContextCurrent(m_Window);
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetScrollCallback(m_Window, SetScrollCallback);

		glfwSetKeyCallback(m_Window, SetKeyCallback);
		glfwSetMouseButtonCallback(m_Window, SetMouseButtonCallback);
		glfwSetCursorPosCallback(m_Window, SetCursorPositionCallback);
		glfwSetScrollCallback(m_Window, SetScrollCallback);

		if (m_Resizeable) {
			glfwSetFramebufferSizeCallback(m_Window, SetWindowResizedCallback);
		}

		glfwSwapInterval(SetVsync ? 1 : 0); // V-Sync
		UpdateWindowSize();
		s_ActiveWindow = this;
	}

	void Window::SetKeyCallback(GLFWwindow* window, int key, int, int action, int) {
		(void)window;
		if (key == GLFW_KEY_UNKNOWN) {
			return;
		}

		switch (action) {
		case GLFW_PRESS:
			Input::OnKeyDown(key);
			break;
		case GLFW_RELEASE:
			Input::OnKeyUp(key);
			break;
		case GLFW_REPEAT:
			Input::OnKeyDown(key);
			break;
		default:
			break;
		}
	}

	void Window::SetMouseButtonCallback(GLFWwindow*, int button, int action, int) {
		switch (action) {
		case GLFW_PRESS:
			Input::OnMouseDown(button);
			break;
		case GLFW_RELEASE:
			Input::OnMouseUp(button);
			break;
		default:
			break;
		}
	}

	void Window::SetCursorPositionCallback(GLFWwindow*, double xPos, double yPos) {
		Input::OnMouseMove(xPos, yPos);
	}

	void Window::SetScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		(void)window;
		(void)xoffset;
		Input::OnScroll(static_cast<float>(yoffset));
	}

	void Window::CenterWindow() {
		Vec2Int screenCenter = GetScreenCenter();
		int windowWidth, windowHeight;
		glfwGetWindowSize(m_Window, &windowWidth, &windowHeight);
		int posX = screenCenter.x - windowWidth / 2;
		int posY = screenCenter.y - windowHeight / 2;
		glfwSetWindowPos(m_Window, posX, posY);
	}
	void Window::FocusWindow() {
		glfwFocusWindow(m_Window);
	}

	void Window::Destroy() {
		glfwDestroyWindow(m_Window);
	}

	Vec2Int Window::GetScreenCenter() const {
		return Vec2Int(k_Mode->width / 2, k_Mode->height / 2);
	}

	void Window::MaximizeWindow(bool reset) {
		if (reset && IsMaximized()) {
			glfwRestoreWindow(m_Window);
		}
		else {
			glfwMaximizeWindow(m_Window);
		}

		glfwGetWindowSize(m_Window, &m_Width, &m_Height);
	}

	void Window::MinimizeWindow() {
		glfwIconifyWindow(m_Window);
		glfwGetWindowSize(m_Window, &m_Width, &m_Height);
	}

	void Window::RestoreWindow() {
		glfwRestoreWindow(m_Window);
	}

	bool Window::IsMaximized() const {
		return glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED);
	}

	bool Window::IsMinimized() const {
		return glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED);
	}

	void Window::SetWindowResizedCallback(GLFWwindow* window, int WIDTH, int HEIGHT) {
		auto _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		_window->m_WindowResized = true;
		_window->m_Width = WIDTH;
		_window->m_Height = HEIGHT;
		_window->UpdateViewport();
	}

	void Window::UpdateViewport() {
		glViewport(0, 0, m_Width, m_Height);
	}

	void Window::UpdateWindowSize() {
		glfwGetWindowSize(m_Window, &m_Width, &m_Height);
	}
}