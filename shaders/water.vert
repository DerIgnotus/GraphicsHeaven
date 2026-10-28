#version 460 core

layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 WorldPos;

void main()
{
    vec4 worldPosition = model * vec4(aPos, 1.0);
    worldPosition.y -= 2.0;
    WorldPos = worldPosition.xyz;

    gl_Position = projection * view * worldPosition;
}
