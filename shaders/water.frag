
#version 460 core

in vec3 WorldPos;
out vec4 FragColor;

uniform float time;

void main()
{
    vec2 p = WorldPos.xz * 3.0f;

    float wave1 = sin(p.x * 0.035 + p.y * 0.018 + time * 0.25);
    float wave2 = sin(p.y * 0.028 - p.x * 0.012 - time * 0.18);

    float waves = wave1 * 0.6 + wave2 * 0.4;

    vec3 deepColor = vec3(0.012, 0.085, 0.12);
    vec3 lightColor = vec3(0.025, 0.15, 0.18);

    float variation = 0.5 + 0.5 * waves;
    vec3 color = mix(deepColor, lightColor, variation * 0.45);

    FragColor = vec4(color, 1.0);
}
