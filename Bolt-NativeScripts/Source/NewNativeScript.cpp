#include <Scripting/NativeScript.hpp>

class NewNativeScript : public Bolt::NativeScript {
public:
    void Start() override
    {
        BT_NATIVE_LOG_INFO("NewNativeScript started!");
    }

    void Update(float dt) override
    {
        float x, y;
        GetPosition(x, y);
        SetPosition(x + 5.0f * dt, y);
    }
};
REGISTER_SCRIPT(NewNativeScript)
