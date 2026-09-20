#version 460 core

out vec4 FragColor;

uniform mat4 invViewProjection;
uniform vec2 screenSize;

void main()
{
    vec2 uv = gl_FragCoord.xy / screenSize;

    vec4 clipPos = vec4(
        uv * 2.0 - 1.0,
        1.0,
        1.0
    );

    vec4 worldPos = invViewProjection * clipPos;

    worldPos /= worldPos.w;

    vec3 viewDir = normalize(worldPos.xyz);

    FragColor = vec4(
        viewDir * 0.5 + 0.5,
        1.0
    );
}