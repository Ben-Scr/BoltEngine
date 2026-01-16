#include "../pch.hpp"
#include "Window.hpp"
#include "Application.hpp"

namespace Bolt {
	Window* Window::s_ActiveWindow = nullptr;
	bool Window::s_IsVsync = true;
	Viewport Window::s_MainViewport;

	void Test(const std::string& t) {

	}

	Window::Window(int width, int height, const std::string& title)
    {
		InitWindow(GLFWWindowProperties{ width , height, title });
	}

	Window::Window(const GLFWWindowProperties& props)
	{
		InitWindow(props);
	}

	void Window::RefreshCallback(GLFWwindow* window) {
		Application* app = Application::GetInstance();

		if (app == nullptr) return;

		app->m_Renderer2D->BeginFrame();
		app->m_GizmoRenderer2D->BeginFrame();
		app->m_GizmoRenderer2D->EndFrame();
		app->m_Renderer2D->EndFrame();

		app->m_Window->SwapBuffers();
	}

	void Window::FocusCallback(GLFWwindow* window, int focused) {
	

		if (focused == 1) {
			
			Application::Pause(false);
			Logger::Message("Focused " + std::to_string(focused));
		}
		else if(focused == 0)  {
			Application::Pause(true);
			Logger::Message("Unfocused " + std::to_string(focused));
		}
	}

	void Window::InitWindow(const GLFWWindowProperties& props) {
		s_MainViewport = Viewport{ props.Width, props.Height };

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_SAMPLES, 8);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


		glfwWindowHint(GLFW_DECORATED, props.Moveable);
		glfwWindowHint(GLFW_RESIZABLE, props.Resizeable);

		m_Monitor = glfwGetPrimaryMonitor();
		k_Mode = glfwGetVideoMode(m_Monitor);


		if (props.Fullscreen) {
			m_Window = glfwCreateWindow(k_Mode->width, k_Mode->height, props.Title.c_str(), nullptr, nullptr);
			MaximizeWindow();
		}
		else {
			m_Window = glfwCreateWindow(s_MainViewport.Width, s_MainViewport.Height, props.Title.c_str(), nullptr, nullptr);
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
		glfwSetWindowFocusCallback(m_Window, FocusCallback);

		if (props.Resizeable) {
			glfwSetFramebufferSizeCallback(m_Window, SetWindowResizedCallback);
		}

		glfwSetWindowRefreshCallback(m_Window, RefreshCallback);

		glfwSwapInterval(SetVsync ? 1 : 0);
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
		m_Window = nullptr;
	}
	void Window::Shutdown() {
		glfwTerminate();
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

		glfwGetWindowSize(m_Window, &s_MainViewport.Width, &s_MainViewport.Height);
	}

	void Window::MinimizeWindow() {
		glfwIconifyWindow(m_Window);
		glfwGetWindowSize(m_Window, &s_MainViewport.Width, &s_MainViewport.Height);
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

	void Window::SetWindowResizedCallback(GLFWwindow* window, int width, int height) {
		Window* _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

		if (_window == nullptr) return;

		_window->s_MainViewport.Width = width;
		_window->s_MainViewport.Height = height;
		_window->UpdateViewport();
	}

	void Window::UpdateViewport() {
		glViewport(0, 0, s_MainViewport.Width, s_MainViewport.Height);
	}

	void Window::UpdateWindowSize() {
		glfwGetWindowSize(m_Window, &s_MainViewport.Width, &s_MainViewport.Height);
	}
}