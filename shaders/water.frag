#version 460 core

in vec3 WorldPos;

out vec4 FragColor;

uniform float seaLevel;

uniform vec3 cameraPos;
uniform float time;

void main()
{
    vec2 uv = WorldPos.xz * 0.035;

    float wave1 = sin(uv.x * 2.0 + time * 0.8)
                * cos(uv.y * 1.6 - time * 0.5);

    float wave2 = sin((uv.x + uv.y) * 3.0 + time * 0.6);

    float ripples = wave1 * 0.5 + wave2 * 0.25;

    vec3 deepWater = vec3(0.015, 0.12, 0.20);
    vec3 shallowWater = vec3(0.04, 0.32, 0.38);

    float depthBlend = clamp(0.45 + ripples * 0.15, 0.0, 1.0);
    vec3 waterColor = mix(deepWater, shallowWater, depthBlend);

    FragColor = vec4(waterColor, 0.88);
}
