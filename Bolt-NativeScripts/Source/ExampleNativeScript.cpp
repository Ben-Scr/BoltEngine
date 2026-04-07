#include <Scripting/NativeScript.hpp>
#include <cmath>

class FloatingMotion : public Bolt::NativeScript {
public:
	void Start() override { m_Time = 0.0f; }
	void Update(float dt) override
	{
		m_Time += dt;
		float x, y;
		GetPosition(x, y);
		SetPosition(x, sinf(m_Time * 2.0f) * 1.5f);
	}
private:
	float m_Time = 0.0f;
};
REGISTER_SCRIPT(FloatingMotion)

class Spinner : public Bolt::NativeScript {
public:
	void Update(float dt) override
	{
		SetRotation(GetRotation() + 3.14159f * dt);
	}
};
REGISTER_SCRIPT(Spinner)

class PatrolMovement : public Bolt::NativeScript {
public:
	void Update(float dt) override
	{
		float x, y;
		GetPosition(x, y);
		m_Distance += 3.0f * dt * m_Dir;
		if (m_Distance > 5.0f || m_Distance < -5.0f) m_Dir *= -1.0f;
		SetPosition(x + 3.0f * dt * m_Dir, y);
	}
private:
	float m_Distance = 0.0f;
	float m_Dir = 1.0f;
};
REGISTER_SCRIPT(PatrolMovement)

// ── DLL exports ────────────────────────────────────────────────────

extern "C" __declspec(dllexport)
void BoltInitialize(void* engineAPI)
{
	Bolt::g_EngineAPI = static_cast<Bolt::NativeEngineAPI*>(engineAPI);
}

extern "C" __declspec(dllexport)
Bolt::NativeScript* BoltCreateScript(const char* className)
{
	return Bolt::NativeScriptRegistry::Create(className);
}

extern "C" __declspec(dllexport)
void BoltDestroyScript(Bolt::NativeScript* script)
{
	delete script;
}
