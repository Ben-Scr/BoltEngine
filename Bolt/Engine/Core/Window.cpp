#include "../pch.hpp"
#include "Window.hpp"
#include "Application.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Graphics/OpenGL.hpp"

#include <glad/glad.h>

namespace Bolt {
	Window* Window::s_ActiveWindow = nullptr;
	bool Window::s_IsVsync = true;
	Viewport Window::s_MainViewport;
	bool Window::s_IsInitialized = false;
	const GLFWvidmode* Window::k_Videomode = nullptr;

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

		if (!app) return;

		if (app->m_Renderer2D) {
			app->m_Renderer2D->BeginFrame();
			app->m_Renderer2D->EndFrame();
		}

		if (app->m_GizmoRenderer2D) {
			app->m_GizmoRenderer2D->BeginFrame();
			app->m_GizmoRenderer2D->EndFrame();
		}

		if (app->m_Window) app->m_Window->SwapBuffers();
	}

	void Window::Initialize() {
		int code = glfwInit();

	//	if (code != GLFW_TRUE)
			//throw BoltException("GLFWException", "GLFW library couldn't initialize, error code \'" + std::to_string(code) + "'\'");

		k_Videomode = glfwGetVideoMode(GetMainMonitor());
		s_IsInitialized = true;
	}

	void Window::Shutdown() {
		glfwTerminate();
	}

	void Window::FocusCallback(GLFWwindow* window, int focused) {
		if ((bool)focused) {
			Application::Pause(false);
			Logger::Message("Focused " + std::to_string(focused));
		}
		else {
			if (!Application::s_RunInBackground)
				Application::Pause(true);
			Logger::Message("Unfocused " + std::to_string(focused));
		}
	}

	void Window::InitWindow(const GLFWWindowProperties& props) {
		//if (!s_IsInitialized)
			//throw NotInitializedException("The Window isn't initialized");

		s_MainViewport = Viewport{ props.Width, props.Height };

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_SAMPLES, 8);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


		glfwWindowHint(GLFW_DECORATED, props.Moveable);
		glfwWindowHint(GLFW_RESIZABLE, props.Resizeable);

		if (props.Fullscreen) {
			m_Window = glfwCreateWindow(k_Videomode->width, k_Videomode->height, props.Title.c_str(), nullptr, nullptr);
			MaximizeWindow();
		}
		else {
			m_Window = glfwCreateWindow(s_MainViewport.Width, s_MainViewport.Height, props.Title.c_str(), nullptr, nullptr);
			CenterWindow();
		}

		//if (!m_Window)
			//throw BoltException("WindowException", "Failed to create window!");

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

	Vec2 Window::GetCursorPosition() const {
		double x, y;
		glfwGetCursorPos(m_Window, &x, &y);
		return Vec2(x, y);
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
	void Window::SetCursorPositionCallback(GLFWwindow* window, double xPos, double yPos) {
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

	Vec2Int Window::GetScreenCenter() const {
		return Vec2Int(k_Videomode->width / 2, k_Videomode->height / 2);
	}
	Vec2 Window::GetWindowCenter() const {
		Vec2Int size = GetSize();
		return Vec2(size.x / 2.f, size.y / 2.f);
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

	void Window::SetFullScreen(bool enabled) {
		bool isFullScreen = IsFullScreen();

		if ((isFullScreen && enabled) || (!isFullScreen && !enabled))
			return;

		if (enabled) {
			m_RestoreSize = GetSize();
			m_RestorePos = GetPosition();
		}

		glfwSetWindowMonitor(
			m_Window,
			enabled ? GetMainMonitor() : NULL,
			0, 0,
			k_Videomode->width, k_Videomode->height,
			k_Videomode->refreshRate
		);

		if (!enabled) {
			SetSize(m_RestoreSize);
			SetPosition(m_RestorePos);
		}
	}

	bool Window::IsFullScreen()const {
		return GetWindowMonitor() != nullptr;
	}

	GLFWmonitor* Window::GetWindowMonitor() const { return glfwGetWindowMonitor(m_Window); }
	GLFWmonitor* Window::GetMainMonitor() { return glfwGetPrimaryMonitor(); }

	void Window::MinimizeWindow() {
		glfwIconifyWindow(m_Window);
		glfwGetWindowSize(m_Window, &s_MainViewport.Width, &s_MainViewport.Height);
	}

	void Window::RestoreWindow() {
		glfwRestoreWindow(m_Window);
	}

	void Window::SetCursorImage(const Texture2D* tex2D) {
		ImageData* imgData = tex2D->GetImageData();
		imgData->Width;
		imgData->Height;

		GLFWimage img;
		img.width = imgData->Width;
		img.height = imgData->Height;
		img.pixels = imgData->Pixels;

		int xhot = 0;
		int yhot = 0;

		GLFWcursor* newCursor = glfwCreateCursor(&img, xhot, yhot);
		if (!newCursor)
			return;

		glfwSetCursor(m_Window, newCursor);

		if (m_Cursor)
			glfwDestroyCursor(m_Cursor);

		m_Cursor = newCursor;
	}

	void Window::SetWindowIcon(const Texture2D* tex2D) {
		//if (tex2D == nullptr)
			//throw NullReferenceException("Texture is null");

		ImageData* imgData = tex2D->GetImageData();
		imgData->FlipVerticalRGBA();

		GLFWimage img;
		img.width = imgData->Width;
		img.height = imgData->Height;
		img.pixels = imgData->Pixels;

		glfwSetWindowIcon(m_Window, 1, &img);
	}

	bool Window::IsMaximized() const {
		return glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED);
	}

	bool Window::IsMinimized() const {
		return glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED);
	}

	void Window::SetWindowResizedCallback(GLFWwindow* window, int width, int height) {
		Window* _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

		if (!_window) return;

		_window->s_MainViewport.Width = width;
		_window->s_MainViewport.Height = height;
		_window->UpdateViewport();
	}

	void Window::UpdateViewport() {
		if (OpenGL::IsInitialized())
		{
			glViewport(0, 0, s_MainViewport.Width, s_MainViewport.Height);
		}
	}

	void Window::UpdateWindowSize() {
		glfwGetWindowSize(m_Window, &s_MainViewport.Width, &s_MainViewport.Height);
	}
}