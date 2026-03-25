#pragma once

class Window;
class SceneManager;
class Renderer2D;
class AudioSystem;
class PhysicsWorld;
//class AssetManager;
//class InputState;
//class TimeState;

struct EngineContext {
    Window& window;
    SceneManager& sceneManager;
    Renderer2D& renderer;
    AudioSystem& audio;
    PhysicsWorld& physics;
    //AssetManager& assets;
    //InputState& input;
    //TimeState& time;
};