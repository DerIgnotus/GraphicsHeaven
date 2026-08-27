#version 460 core

out vec4 FragColor;

in vec2 TexCoord;

void main()
{
    //FragColor = vec4(result, 1.0);
    FragColor = vec4(0.5, 0.5, 1.0, 1.0);
}