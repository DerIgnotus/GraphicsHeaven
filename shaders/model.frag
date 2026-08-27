#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 baseColor;
uniform sampler2D textureSampler;
uniform bool hasTexture;

//uniform vec4 colorUni;

void main()
{
    vec4 color = baseColor;

    if (hasTexture)
        color *= texture(textureSampler, TexCoords);

    FragColor = color;
    //FragColor = colorUni;
}