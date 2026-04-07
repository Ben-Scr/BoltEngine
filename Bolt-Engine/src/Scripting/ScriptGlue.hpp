#pragma once
#include <cstdint>

namespace Bolt {

	/// Layout must match C# NativeBindingsStruct exactly (Sequential, blittable).
	struct NativeBindings
	{
		float  (*Application_GetDeltaTime)();
		float  (*Application_GetElapsedTime)();
		int    (*Application_GetScreenWidth)();
		int    (*Application_GetScreenHeight)();

		void   (*Log_Trace)(const char* message);
		void   (*Log_Info)(const char* message);
		void   (*Log_Warn)(const char* message);
		void   (*Log_Error)(const char* message);

		int    (*Input_GetKey)(int keyCode);
		int    (*Input_GetKeyDown)(int keyCode);
		int    (*Input_GetKeyUp)(int keyCode);
		int    (*Input_GetMouseButton)(int button);
		int    (*Input_GetMouseButtonDown)(int button);
		void   (*Input_GetMousePosition)(float* outX, float* outY);

		int      (*Entity_IsValid)(uint64_t entityID);
		uint64_t (*Entity_FindByName)(const char* name);
		void     (*Entity_Destroy)(uint64_t entityID);
		uint64_t (*Entity_Create)(const char* name);

		const char* (*NameComponent_GetName)(uint64_t entityID);
		void        (*NameComponent_SetName)(uint64_t entityID, const char* name);

		void  (*Transform2D_GetPosition)(uint64_t entityID, float* outX, float* outY);
		void  (*Transform2D_SetPosition)(uint64_t entityID, float x, float y);
		float (*Transform2D_GetRotation)(uint64_t entityID);
		void  (*Transform2D_SetRotation)(uint64_t entityID, float rotation);
		void  (*Transform2D_GetScale)(uint64_t entityID, float* outX, float* outY);
		void  (*Transform2D_SetScale)(uint64_t entityID, float x, float y);

		void (*SpriteRenderer_GetColor)(uint64_t entityID, float* r, float* g, float* b, float* a);
		void (*SpriteRenderer_SetColor)(uint64_t entityID, float r, float g, float b, float a);

		float (*Camera2D_GetOrthographicSize)(uint64_t entityID);
		void  (*Camera2D_SetOrthographicSize)(uint64_t entityID, float size);
		float (*Camera2D_GetZoom)(uint64_t entityID);
		void  (*Camera2D_SetZoom)(uint64_t entityID, float zoom);

		void  (*Rigidbody2D_ApplyForce)(uint64_t entityID, float forceX, float forceY, int wake);
		void  (*Rigidbody2D_ApplyImpulse)(uint64_t entityID, float impulseX, float impulseY, int wake);
		void  (*Rigidbody2D_GetLinearVelocity)(uint64_t entityID, float* outX, float* outY);
		void  (*Rigidbody2D_SetLinearVelocity)(uint64_t entityID, float x, float y);
		float (*Rigidbody2D_GetAngularVelocity)(uint64_t entityID);
		void  (*Rigidbody2D_SetAngularVelocity)(uint64_t entityID, float velocity);

		int (*Physics2D_Raycast)(float originX, float originY, float dirX, float dirY, float distance,
		                         uint64_t* hitEntityID, float* hitX, float* hitY, float* hitNormalX, float* hitNormalY);
	};

	/// Layout must match C# ManagedCallbacksStruct exactly.
	struct ManagedCallbacks
	{
		int32_t (*CreateScriptInstance)(const char* className, uint64_t entityID);
		void    (*DestroyScriptInstance)(int32_t handle);
		void    (*InvokeStart)(int32_t handle);
		void    (*InvokeUpdate)(int32_t handle);
		void    (*InvokeOnDestroy)(int32_t handle);
		int     (*ClassExists)(const char* className);
		int     (*LoadUserAssembly)(const char* path);
		void    (*UnloadUserAssembly)();
	};

} // namespace Bolt
