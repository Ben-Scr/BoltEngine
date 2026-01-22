#version 330 core

layout (location = 0) in vec2 aPos; 
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;

uniform mat4 uMVP;
uniform vec2 uSpritePos;          
uniform float uRotation;          
uniform vec2 uScale;


uniform vec2 uUVOffset = vec2(0.0);
uniform vec2 uUVScale  = vec2(1.0);
uniform bvec2 uFlip = bvec2(false, false);

out vec2 vUV;
out vec4 vColor;

void main()
{
    float c = cos(uRotation);
    float s = sin(uRotation);
    mat2 R = mat2(c, -s, 
                  s,  c);

    vec2 worldPos = (R * (aPos * uScale)) + uSpritePos;

    gl_Position = uMVP * vec4(worldPos, 0.0, 1.0);

    vec2 uv = aUV;
    if (uFlip.x) uv.x = 1.0 - uv.x;
    if (uFlip.y) uv.y = 1.0 - uv.y;

    vUV    = uUVOffset + uv * uUVScale;
    vColor = aColor;
}