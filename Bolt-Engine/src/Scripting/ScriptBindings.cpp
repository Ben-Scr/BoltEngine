#include "pch.hpp"
#include "Scripting/ScriptBindings.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Core/Application.hpp"
#include "Core/Input.hpp"
#include "Core/Time.hpp"
#include "Core/Window.hpp"
#include "Core/Log.hpp"
#include "Scene/Scene.hpp"
#include "Components/General/Transform2DComponent.hpp"
#include "Components/General/NameComponent.hpp"
#include "Components/Graphics/SpriteRendererComponent.hpp"
#include "Components/Graphics/Camera2DComponent.hpp"
#include "Components/Physics/Rigidbody2DComponent.hpp"
#include "Physics/Physics2D.hpp"

namespace Bolt {

	// ── Helpers ─────────────────────────────────────────────────────────

	static EntityHandle ToEntityHandle(uint64_t id)
	{
		return static_cast<EntityHandle>(static_cast<uint32_t>(id));
	}

	static uint64_t FromEntityHandle(EntityHandle handle)
	{
		return static_cast<uint64_t>(static_cast<uint32_t>(handle));
	}

	static Scene* GetScene()
	{
		Scene* scene = ScriptEngine::GetScene();
		if (!scene)
		{
			BT_CORE_ERROR_TAG("ScriptBindings", "No active scene set on ScriptEngine");
		}
		return scene;
	}

	// Thread-local buffer for returning strings to managed code
	static thread_local std::string s_StringReturnBuffer;

	// ── Application Bindings ────────────────────────────────────────────

	static float Bolt_Application_GetDeltaTime()
	{
		auto* app = Application::GetInstance();
		return app ? app->GetTime().GetDeltaTime() : 0.0f;
	}

	static float Bolt_Application_GetTime()
	{
		auto* app = Application::GetInstance();
		return app ? app->GetTime().GetElapsedTime() : 0.0f;
	}

	static int Bolt_Application_GetScreenWidth()
	{
		auto* window = Application::GetWindow();
		return window ? window->GetWidth() : 0;
	}

	static int Bolt_Application_GetScreenHeight()
	{
		auto* window = Application::GetWindow();
		return window ? window->GetHeight() : 0;
	}

	// ── Log Bindings ────────────────────────────────────────────────────

	static void Bolt_Log_Trace(const char* message)
	{
		Log::PrintMessageTag(Log::Type::Client, Log::Level::Trace, "Script", message);
	}

	static void Bolt_Log_Info(const char* message)
	{
		Log::PrintMessageTag(Log::Type::Client, Log::Level::Info, "Script", message);
	}

	static void Bolt_Log_Warn(const char* message)
	{
		Log::PrintMessageTag(Log::Type::Client, Log::Level::Warn, "Script", message);
	}

	static void Bolt_Log_Error(const char* message)
	{
		Log::PrintMessageTag(Log::Type::Client, Log::Level::Error, "Script", message);
	}

	// ── Input Bindings ──────────────────────────────────────────────────

	static int Bolt_Input_GetKey(int keyCode)
	{
		auto* app = Application::GetInstance();
		return app ? (app->GetInput().GetKey(static_cast<KeyCode>(keyCode)) ? 1 : 0) : 0;
	}

	static int Bolt_Input_GetKeyDown(int keyCode)
	{
		auto* app = Application::GetInstance();
		return app ? (app->GetInput().GetKeyDown(static_cast<KeyCode>(keyCode)) ? 1 : 0) : 0;
	}

	static int Bolt_Input_GetKeyUp(int keyCode)
	{
		auto* app = Application::GetInstance();
		return app ? (app->GetInput().GetKeyUp(static_cast<KeyCode>(keyCode)) ? 1 : 0) : 0;
	}

	static int Bolt_Input_GetMouseButton(int button)
	{
		auto* app = Application::GetInstance();
		return app ? (app->GetInput().GetMouse(static_cast<MouseButton>(button)) ? 1 : 0) : 0;
	}

	static int Bolt_Input_GetMouseButtonDown(int button)
	{
		auto* app = Application::GetInstance();
		return app ? (app->GetInput().GetMouseDown(static_cast<MouseButton>(button)) ? 1 : 0) : 0;
	}

	static void Bolt_Input_GetMousePosition(float* outX, float* outY)
	{
		auto* app = Application::GetInstance();
		if (app)
		{
			Vec2 pos = app->GetInput().GetMousePosition();
			*outX = pos.x;
			*outY = pos.y;
		}
		else
		{
			*outX = 0.0f;
			*outY = 0.0f;
		}
	}

	// ── Entity Bindings ─────────────────────────────────────────────────

	static int Bolt_Entity_IsValid(uint64_t entityID)
	{
		Scene* scene = GetScene();
		if (!scene) return 0;
		return scene->IsValid(ToEntityHandle(entityID)) ? 1 : 0;
	}

	static uint64_t Bolt_Entity_FindByName(const char* name)
	{
		Scene* scene = GetScene();
		if (!scene) return 0;

		std::string targetName(name);
		auto& registry = scene->GetRegistry();
		auto view = registry.view<NameComponent>();
		for (auto [entity, nameComp] : view.each())
		{
			if (nameComp.Name == targetName)
			{
				return FromEntityHandle(entity);
			}
		}

		return 0;
	}

	static void Bolt_Entity_Destroy(uint64_t entityID)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (scene->IsValid(handle))
		{
			scene->DestroyEntity(handle);
		}
	}

	// ── NameComponent Bindings ──────────────────────────────────────────

	static const char* Bolt_NameComponent_GetName(uint64_t entityID)
	{
		Scene* scene = GetScene();
		if (!scene) { s_StringReturnBuffer.clear(); return s_StringReturnBuffer.c_str(); }

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<NameComponent>(handle))
		{
			s_StringReturnBuffer.clear();
			return s_StringReturnBuffer.c_str();
		}

		s_StringReturnBuffer = scene->GetComponent<NameComponent>(handle).Name;
		return s_StringReturnBuffer.c_str();
	}

	static void Bolt_NameComponent_SetName(uint64_t entityID, const char* name)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<NameComponent>(handle)) return;

		scene->GetComponent<NameComponent>(handle).Name = name;
	}

	// ── Transform2DComponent Bindings ───────────────────────────────────

	static void Bolt_Transform2D_GetPosition(uint64_t entityID, float* outX, float* outY)
	{
		Scene* scene = GetScene();
		if (!scene) { *outX = 0; *outY = 0; return; }

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Transform2DComponent>(handle)) { *outX = 0; *outY = 0; return; }

		auto& comp = scene->GetComponent<Transform2DComponent>(handle);
		*outX = comp.Position.x;
		*outY = comp.Position.y;
	}

	static void Bolt_Transform2D_SetPosition(uint64_t entityID, float x, float y)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Transform2DComponent>(handle)) return;

		auto& comp = scene->GetComponent<Transform2DComponent>(handle);
		comp.Position = { x, y };
	}

	static float Bolt_Transform2D_GetRotation(uint64_t entityID)
	{
		Scene* scene = GetScene();
		if (!scene) return 0.0f;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Transform2DComponent>(handle)) return 0.0f;

		return scene->GetComponent<Transform2DComponent>(handle).Rotation;
	}

	static void Bolt_Transform2D_SetRotation(uint64_t entityID, float rotation)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Transform2DComponent>(handle)) return;

		scene->GetComponent<Transform2DComponent>(handle).Rotation = rotation;
	}

	static void Bolt_Transform2D_GetScale(uint64_t entityID, float* outX, float* outY)
	{
		Scene* scene = GetScene();
		if (!scene) { *outX = 1; *outY = 1; return; }

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Transform2DComponent>(handle)) { *outX = 1; *outY = 1; return; }

		auto& comp = scene->GetComponent<Transform2DComponent>(handle);
		*outX = comp.Scale.x;
		*outY = comp.Scale.y;
	}

	static void Bolt_Transform2D_SetScale(uint64_t entityID, float x, float y)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Transform2DComponent>(handle)) return;

		scene->GetComponent<Transform2DComponent>(handle).Scale = { x, y };
	}

	// ── SpriteRendererComponent Bindings ────────────────────────────────

	static void Bolt_SpriteRenderer_GetColor(uint64_t entityID, float* r, float* g, float* b, float* a)
	{
		Scene* scene = GetScene();
		if (!scene) { *r = 1; *g = 1; *b = 1; *a = 1; return; }

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<SpriteRendererComponent>(handle)) { *r = 1; *g = 1; *b = 1; *a = 1; return; }

		auto& comp = scene->GetComponent<SpriteRendererComponent>(handle);
		*r = comp.Color.r;
		*g = comp.Color.g;
		*b = comp.Color.b;
		*a = comp.Color.a;
	}

	static void Bolt_SpriteRenderer_SetColor(uint64_t entityID, float r, float g, float b, float a)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<SpriteRendererComponent>(handle)) return;

		auto& comp = scene->GetComponent<SpriteRendererComponent>(handle);
		comp.Color = { r, g, b, a };
	}

	// ── Camera2DComponent Bindings ──────────────────────────────────────

	static float Bolt_Camera2D_GetOrthographicSize(uint64_t entityID)
	{
		Scene* scene = GetScene();
		if (!scene) return 5.0f;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Camera2DComponent>(handle)) return 5.0f;

		return scene->GetComponent<Camera2DComponent>(handle).GetOrthographicSize();
	}

	static void Bolt_Camera2D_SetOrthographicSize(uint64_t entityID, float size)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Camera2DComponent>(handle)) return;

		scene->GetComponent<Camera2DComponent>(handle).SetOrthographicSize(size);
	}

	static float Bolt_Camera2D_GetZoom(uint64_t entityID)
	{
		Scene* scene = GetScene();
		if (!scene) return 1.0f;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Camera2DComponent>(handle)) return 1.0f;

		return scene->GetComponent<Camera2DComponent>(handle).GetZoom();
	}

	static void Bolt_Camera2D_SetZoom(uint64_t entityID, float zoom)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Camera2DComponent>(handle)) return;

		scene->GetComponent<Camera2DComponent>(handle).SetZoom(zoom);
	}

	// ── Rigidbody2DComponent Bindings ───────────────────────────────────

	static void Bolt_Rigidbody2D_ApplyForce(uint64_t entityID, float forceX, float forceY, int wake)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Rigidbody2DComponent>(handle)) return;

		auto& rb = scene->GetComponent<Rigidbody2DComponent>(handle);
		b2BodyId bodyId = rb.GetBodyHandle();
		if (b2Body_IsValid(bodyId))
		{
			b2Body_ApplyForceToCenter(bodyId, { forceX, forceY }, wake != 0);
		}
	}

	static void Bolt_Rigidbody2D_ApplyImpulse(uint64_t entityID, float impulseX, float impulseY, int wake)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Rigidbody2DComponent>(handle)) return;

		auto& rb = scene->GetComponent<Rigidbody2DComponent>(handle);
		b2BodyId bodyId = rb.GetBodyHandle();
		if (b2Body_IsValid(bodyId))
		{
			b2Body_ApplyLinearImpulseToCenter(bodyId, { impulseX, impulseY }, wake != 0);
		}
	}

	static void Bolt_Rigidbody2D_GetLinearVelocity(uint64_t entityID, float* outX, float* outY)
	{
		Scene* scene = GetScene();
		if (!scene) { *outX = 0; *outY = 0; return; }

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Rigidbody2DComponent>(handle)) { *outX = 0; *outY = 0; return; }

		Vec2 vel = scene->GetComponent<Rigidbody2DComponent>(handle).GetVelocity();
		*outX = vel.x;
		*outY = vel.y;
	}

	static void Bolt_Rigidbody2D_SetLinearVelocity(uint64_t entityID, float x, float y)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Rigidbody2DComponent>(handle)) return;

		scene->GetComponent<Rigidbody2DComponent>(handle).SetVelocity({ x, y });
	}

	static float Bolt_Rigidbody2D_GetAngularVelocity(uint64_t entityID)
	{
		Scene* scene = GetScene();
		if (!scene) return 0.0f;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Rigidbody2DComponent>(handle)) return 0.0f;

		return scene->GetComponent<Rigidbody2DComponent>(handle).GetAngularVelocity();
	}

	static void Bolt_Rigidbody2D_SetAngularVelocity(uint64_t entityID, float velocity)
	{
		Scene* scene = GetScene();
		if (!scene) return;

		EntityHandle handle = ToEntityHandle(entityID);
		if (!scene->HasComponent<Rigidbody2DComponent>(handle)) return;

		scene->GetComponent<Rigidbody2DComponent>(handle).SetAngularVelocity(velocity);
	}

	// ── Physics2D Bindings ──────────────────────────────────────────────

	static int Bolt_Physics2D_Raycast(
		float originX, float originY,
		float dirX, float dirY,
		float distance,
		uint64_t* hitEntityID,
		float* hitX, float* hitY,
		float* hitNormalX, float* hitNormalY)
	{
		*hitEntityID = 0;
		*hitX = 0; *hitY = 0;
		*hitNormalX = 0; *hitNormalY = 0;

		// TODO: Implement when PhysicsSystem2D::RayCast is available
		return 0;
	}

	// ── Registration ────────────────────────────────────────────────────

	void ScriptBindings::PopulateNativeBindings(NativeBindings& b)
	{
		// Application
		b.Application_GetDeltaTime = &Bolt_Application_GetDeltaTime;
		b.Application_GetElapsedTime = &Bolt_Application_GetTime;
		b.Application_GetScreenWidth = &Bolt_Application_GetScreenWidth;
		b.Application_GetScreenHeight = &Bolt_Application_GetScreenHeight;

		// Log
		b.Log_Trace = &Bolt_Log_Trace;
		b.Log_Info = &Bolt_Log_Info;
		b.Log_Warn = &Bolt_Log_Warn;
		b.Log_Error = &Bolt_Log_Error;

		// Input
		b.Input_GetKey = &Bolt_Input_GetKey;
		b.Input_GetKeyDown = &Bolt_Input_GetKeyDown;
		b.Input_GetKeyUp = &Bolt_Input_GetKeyUp;
		b.Input_GetMouseButton = &Bolt_Input_GetMouseButton;
		b.Input_GetMouseButtonDown = &Bolt_Input_GetMouseButtonDown;
		b.Input_GetMousePosition = &Bolt_Input_GetMousePosition;

		// Entity
		b.Entity_IsValid = &Bolt_Entity_IsValid;
		b.Entity_FindByName = &Bolt_Entity_FindByName;
		b.Entity_Destroy = &Bolt_Entity_Destroy;

		// NameComponent
		b.NameComponent_GetName = &Bolt_NameComponent_GetName;
		b.NameComponent_SetName = &Bolt_NameComponent_SetName;

		// Transform2D
		b.Transform2D_GetPosition = &Bolt_Transform2D_GetPosition;
		b.Transform2D_SetPosition = &Bolt_Transform2D_SetPosition;
		b.Transform2D_GetRotation = &Bolt_Transform2D_GetRotation;
		b.Transform2D_SetRotation = &Bolt_Transform2D_SetRotation;
		b.Transform2D_GetScale = &Bolt_Transform2D_GetScale;
		b.Transform2D_SetScale = &Bolt_Transform2D_SetScale;

		// SpriteRenderer
		b.SpriteRenderer_GetColor = &Bolt_SpriteRenderer_GetColor;
		b.SpriteRenderer_SetColor = &Bolt_SpriteRenderer_SetColor;

		// Camera2D
		b.Camera2D_GetOrthographicSize = &Bolt_Camera2D_GetOrthographicSize;
		b.Camera2D_SetOrthographicSize = &Bolt_Camera2D_SetOrthographicSize;
		b.Camera2D_GetZoom = &Bolt_Camera2D_GetZoom;
		b.Camera2D_SetZoom = &Bolt_Camera2D_SetZoom;

		// Rigidbody2D
		b.Rigidbody2D_ApplyForce = &Bolt_Rigidbody2D_ApplyForce;
		b.Rigidbody2D_ApplyImpulse = &Bolt_Rigidbody2D_ApplyImpulse;
		b.Rigidbody2D_GetLinearVelocity = &Bolt_Rigidbody2D_GetLinearVelocity;
		b.Rigidbody2D_SetLinearVelocity = &Bolt_Rigidbody2D_SetLinearVelocity;
		b.Rigidbody2D_GetAngularVelocity = &Bolt_Rigidbody2D_GetAngularVelocity;
		b.Rigidbody2D_SetAngularVelocity = &Bolt_Rigidbody2D_SetAngularVelocity;

		// Physics2D
		b.Physics2D_Raycast = &Bolt_Physics2D_Raycast;

		BT_CORE_INFO_TAG("ScriptBindings", "All native bindings populated ({} function pointers)", 35);
	}

} // namespace Bolt
