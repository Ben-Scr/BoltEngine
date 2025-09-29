// sprite2d.vert
#version 330 core

layout (location = 0) in vec2 aPos;     // Quad-Position in Sprite-Lokalkoordinaten (z.B. (-0.5,-0.5) .. (0.5,0.5))
layout (location = 1) in vec2 aUV;      // Basis-UVs (0..1 über das Quad)
layout (location = 2) in vec4 aColor;   // Per-Vertex oder per-Instance Farbe (tint)

uniform mat4 uMVP;                       // Model * View * Projection (2D: Ortho)
uniform vec2 uSpritePos;                 // Weltposition des Sprites (x,y)
uniform float uRotation;                 // Rotation in Radiant (um z)
uniform vec2 uScale;                     // Sprite-Skalierung

// Sprite-Sheet / UV-Steuerung
uniform vec2 uUVOffset = vec2(0.0);      // Offset innerhalb der Textur (z.B. Kachel-Start)
uniform vec2 uUVScale  = vec2(1.0);      // Größe des Ausschnitts relativ zur Textur
uniform bvec2 uFlip = bvec2(false, false); // Flip X/Y

out vec2 vUV;
out vec4 vColor;

void main()
{
    // 2D-Transformation (Rot+Scale) um den Ursprung
    float c = cos(uRotation);
    float s = sin(uRotation);
    mat2 R = mat2(c, -s,
                  s,  c);

    vec2 worldPos = (R * (aPos * uScale)) + uSpritePos;

    gl_Position = uMVP * vec4(worldPos, 0.0, 1.0);

    // UV transformieren für Atlas-Ausschnitte + optionales Flip
    vec2 uv = aUV;
    if (uFlip.x) uv.x = 1.0 - uv.x;
    if (uFlip.y) uv.y = 1.0 - uv.y;

    vUV    = uUVOffset + uv * uUVScale;
    vColor = aColor;
}