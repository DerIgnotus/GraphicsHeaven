#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aTexCoords;

layout(location = 4) in vec3 instancePosition;
layout(location = 5) in float instanceRotation;
layout(location = 6) in float instanceScale;
layout(location = 7) in float instanceColorVariation;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;
out vec2 TexCoords;

void main()
{
    float c = cos(instanceRotation);
    float s = sin(instanceRotation);

    mat3 rotation = mat3(
         c, 0.0, -s,
         0.0, 1.0,  0.0,
         s, 0.0,  c
    );

    vec3 localPosition =
        rotation * (aPosition * instanceScale)
        + instancePosition;

    vec4 worldPosition = model * vec4(localPosition, 1.0);

    FragPos = worldPosition.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(model) * rotation));

    Normal = normalize(normalMatrix * aNormal);

    float variation = mix(0.78, 1.18, instanceColorVariation);
    Color = aColor * variation;
    TexCoords = aTexCoords;

    gl_Position = projection * view * worldPosition;
}
