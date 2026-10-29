#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 sunDir;
uniform sampler2D grassTexture;

void main()
{
    vec4 grass = texture(grassTexture, TexCoords);

    if (grass.a < 0.5)
    {
        discard;
    }

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(sunDir);

    // Match the terrain shader's day/night cycle.
    float daylight = smoothstep(0.025, 0.15, sunDir.y);

    float diffuse = max(dot(norm, lightDir), 0.0);

    vec3 baseColor = grass.rgb * Color;

    // Same ambient and direct-light levels as the terrain.
    //vec3 ambient = Color * mix(0.025, 0.2, daylight);
    //vec3 directLight = Color * diffuse * daylight;

    vec3 ambient = baseColor * mix(0.025, 0.2, daylight);
    vec3 directLight = baseColor * diffuse * daylight;

    FragColor = vec4(ambient + directLight, 1.0);
}
