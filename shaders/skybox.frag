#version 460 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{   
    vec3 hdrColor = texture(skybox, TexCoords).rgb;
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

    mapped = pow(mapped, vec3(1.0 / 2.2));
    
    FragColor = vec4(mapped, 1.0);
}