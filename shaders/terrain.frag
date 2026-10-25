#version 460 core

in vec2 fragTexCoords;

out vec4 FragColor;

void main()
{
    float height = fragTexCoords.y;
    vec3 color;

    if (height < 0.2f) {
        color = vec3(0.1f, 0.3f, 0.7f); 
    } 
    else if (height < 0.3f) {
        color = vec3(0.85f, 0.8f, 0.5f); 
    } 
    else if (height < 0.7f) {
        color = vec3(0.15f, 0.5f, 0.15f);
    } 
    else if (height < 0.85f) {
        color = vec3(0.4f, 0.4f, 0.4f); 
    } 
    else {
        color = vec3(0.9f, 0.9f, 0.9f); 
    }

    FragColor = vec4(color, 1.0f);
}