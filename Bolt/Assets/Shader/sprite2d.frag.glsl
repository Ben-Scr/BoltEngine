// sprite2d.frag
#version 330 core

in vec2 vUV;
in vec4 vColor;

uniform sampler2D uTexture;
uniform bool uPremultipliedAlpha = false; // falls deine Textur vor-multipliziert ist

// optionales Soft-Alpha-Testen (Ausschneiden von voll transparenten Pixeln)
uniform float uAlphaCutoff = 0.0;

out vec4 fragColor;

void main()
{
    vec4 texel = texture(uTexture, vUV);

    // Alpha-Cut
    if (texel.a <= uAlphaCutoff)
        discard;

    if (uPremultipliedAlpha) {
        // Bei PMA ist rgb bereits * alpha in der Textur
        fragColor = texel * vec4(vColor.rgb, 1.0);
        fragColor.a = texel.a * vColor.a;
    } else {
        // Nicht-PMA: erst modulieren, dann Alpha anwenden
        vec3 rgb = texel.rgb * vColor.rgb;
        float a  = texel.a * vColor.a;
        fragColor = vec4(rgb, a);
    }
}
