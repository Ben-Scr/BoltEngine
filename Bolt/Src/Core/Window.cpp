#include "../pch.hpp"
#include "Window.hpp"
#include "Application.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Graphics/OpenGL.hpp"

#include <glad/glad.h>

namespace Bolt {
	Window* Window::s_ActiveWindow = nullptr;
	bool Window::s_IsVsync = true;
	std::shared_ptr<Viewport> Window::s_MainViewport = nullptr;
	bool Window::s_IsInitialized = false;
	const GLFWvidmode* Window::k_Videomode = nullptr;

	Window::Window(int width, int height, const std::string& title)
	{
		CreateWindow(GLFWWindowProperties{ width , height, title });
	}

	Window::Window(const GLFWWindowProperties& props)
	{
		CreateWindow(props);
	}

	void Window::RefreshCallback(GLFWwindow* window) {
		Application* app = Application::GetInstance();

		if (!app) return;

		if (app->m_Renderer2D) {
			app->m_Renderer2D->BeginFrame();
			app->m_Renderer2D->EndFrame();
		}

		if (app->m_ImGuiRenderer) {
			app->m_ImGuiRenderer->BeginFrame();
			app->m_ImGuiRenderer->EndFrame();
		}

		if (app->m_GizmoRenderer2D) {
			app->m_GizmoRenderer2D->BeginFrame();
			app->m_GizmoRenderer2D->EndFrame();
		}

		if (app->m_Window) app->m_Window->SwapBuffers();
	}

	void Window::Initialize() {
		int code = glfwInit();

		BOLT_RETURN_IF(code != GLFW_TRUE, BoltErrorCode::Undefined, "GLFW library couldn't initialize, error code " + StringHelper::WrapWith(std::to_string(code), '\''));

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

	void Window::CreateWindow(const GLFWWindowProperties& props) {
		BOLT_RETURN_IF(!s_IsInitialized, BoltErrorCode::NotInitialized, "The Window isn't initialized");

		s_MainViewport = std::make_shared<Viewport>(props.Width, props.Height);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_SAMPLES, 8);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


		glfwWindowHint(GLFW_DECORATED, props.Moveable);
		glfwWindowHint(GLFW_RESIZABLE, props.Resizeable);

		if (props.Fullscreen) {
			m_GLFWwindow = glfwCreateWindow(k_Videomode->width, k_Videomode->height, props.Title.c_str(), nullptr, nullptr);
			MaximizeWindow();
		}
		else {
			m_GLFWwindow = glfwCreateWindow(s_MainViewport->Width, s_MainViewport->Height, props.Title.c_str(), nullptr, nullptr);
			CenterWindow();
		}

		BOLT_RETURN_IF(!m_GLFWwindow, BoltErrorCode::Undefined, "Failed to create window!");

		glfwMakeContextCurrent(m_GLFWwindow);
		glfwSetWindowUserPointer(m_GLFWwindow, this);
		glfwSetScrollCallback(m_GLFWwindow, SetScrollCallback);

		glfwSetKeyCallback(m_GLFWwindow, SetKeyCallback);
		glfwSetMouseButtonCallback(m_GLFWwindow, SetMouseButtonCallback);
		glfwSetCursorPosCallback(m_GLFWwindow, SetCursorPositionCallback);
		glfwSetScrollCallback(m_GLFWwindow, SetScrollCallback);
		glfwSetWindowFocusCallback(m_GLFWwindow, FocusCallback);

		if (props.Resizeable) {
			glfwSetFramebufferSizeCallback(m_GLFWwindow, SetWindowResizedCallback);
		}

		glfwSetWindowRefreshCallback(m_GLFWwindow, RefreshCallback);

		glfwSwapInterval(SetVsync ? 1 : 0);

		if (!s_ActiveWindow)
			s_ActiveWindow = this;
	}
	void Window::Destroy() {
		glfwDestroyWindow(m_GLFWwindow);
		m_GLFWwindow = nullptr;
	}

	void Window::CenterWindow() {
		Vec2Int screenCenter = GetScreenCenter();
		int windowWidth, windowHeight;
		glfwGetWindowSize(m_GLFWwindow, &windowWidth, &windowHeight);
		int posX = screenCenter.x - windowWidth / 2;
		int posY = screenCenter.y - windowHeight / 2;
		glfwSetWindowPos(m_GLFWwindow, posX, posY);
	}
	void Window::Focus() {
		glfwFocusWindow(m_GLFWwindow);
	}
	void Window::MaximizeWindow(bool reset) {
		if (reset && IsMaximized()) {
			glfwRestoreWindow(m_GLFWwindow);
		}
		else {
			glfwMaximizeWindow(m_GLFWwindow);
		}

		glfwGetWindowSize(m_GLFWwindow, &s_MainViewport->Width, &s_MainViewport->Height);
	}
	void Window::MinimizeWindow() {
		glfwIconifyWindow(m_GLFWwindow);
		glfwGetWindowSize(m_GLFWwindow, &s_MainViewport->Width, &s_MainViewport->Height);
	}
	void Window::RestoreWindow() {
		glfwRestoreWindow(m_GLFWwindow);
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
	void Window::SetFullScreen(bool enabled) {
		bool isFullScreen = IsFullScreen();

		if ((isFullScreen && enabled) || (!isFullScreen && !enabled))
			return;

		if (enabled) {
			m_RestoreSize = GetSize();
			m_RestorePos = GetPosition();
		}

		glfwSetWindowMonitor(
			m_GLFWwindow,
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
	void Window::SetCursorLocked(bool enabled) {
		glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, enabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}
	void Window::SetCursorHidden(bool enabled) {
		glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, enabled ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
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

		glfwSetCursor(m_GLFWwindow, newCursor);

		if (m_Cursor)
			glfwDestroyCursor(m_Cursor);

		m_Cursor = newCursor;
	}
	void Window::SetWindowIcon(const Texture2D* tex2D) {
		BOLT_RETURN_IF(!tex2D, BoltErrorCode::NullReference, "Texture is null");

		ImageData* imgData = tex2D->GetImageData();

		BOLT_RETURN_IF(!imgData, BoltErrorCode::NullReference, "Image data is null");
		imgData->FlipVerticalRGBA();

		GLFWimage img;
		img.width = imgData->Width;
		img.height = imgData->Height;
		img.pixels = imgData->Pixels;

		glfwSetWindowIcon(m_GLFWwindow, 1, &img);
	}

	void Window::SetWindowResizedCallback(GLFWwindow* window, int width, int height) {
		Window* _window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

		if (!_window) return;

		_window->s_MainViewport->Width = width;
		_window->s_MainViewport->Height = height;
		_window->UpdateViewport();
	}

	Vec2 Window::GetCursorPosition() const {
		double x, y;
		glfwGetCursorPos(m_GLFWwindow, &x, &y);
		return Vec2(x, y);
	}
	Vec2Int Window::GetScreenCenter() const {
		return Vec2Int(k_Videomode->width / 2, k_Videomode->height / 2);
	}
	Vec2 Window::GetWindowCenter() const {
		Vec2Int size = GetSize();
		return Vec2(size.x / 2.f, size.y / 2.f);
	}
	GLFWmonitor* Window::GetWindowMonitor() const { return glfwGetWindowMonitor(m_GLFWwindow); }
	GLFWmonitor* Window::GetMainMonitor() { return glfwGetPrimaryMonitor(); }

	bool Window::IsMaximized() const {
		return glfwGetWindowAttrib(m_GLFWwindow, GLFW_MAXIMIZED);
	}
	bool Window::IsMinimized() const {
		return glfwGetWindowAttrib(m_GLFWwindow, GLFW_ICONIFIED);
	}
	bool Window::IsFullScreen()const {
		return GetWindowMonitor() != nullptr;
	}


	void Window::UpdateViewport() {
		glfwGetWindowSize(m_GLFWwindow, &s_MainViewport->Width, &s_MainViewport->Height);

		if (OpenGL::IsInitialized())
		{
			glViewport(0, 0, s_MainViewport->Width, s_MainViewport->Height);
		}
	}
}