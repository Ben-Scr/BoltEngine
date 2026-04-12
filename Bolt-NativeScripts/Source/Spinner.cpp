#include <Scripting/NativeScript.hpp>

class Spinner : public Bolt::NativeScript {
public:
	void Update(float dt) override {
		SetRotation(GetRotation() + 3.14159f * dt);
	}
};

REGISTER_SCRIPT(Spinner)
