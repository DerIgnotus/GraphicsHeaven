
#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 fragTexCoords;

out vec4 FragColor;

uniform vec3 sunDir;

uniform sampler2D sandTexture;
uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D snowTexture;

uniform float textureScale = 0.05;

uniform float waterLevel;

void main()
{
    if (FragPos.y < waterLevel)
        discard;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(sunDir);

    vec2 uv = FragPos.xz * textureScale;

    vec3 sand = texture(sandTexture, uv).rgb;
    vec3 grass = texture(grassTexture, uv).rgb;
    vec3 rock = texture(rockTexture, uv).rgb;
    vec3 snow = texture(snowTexture, uv).rgb;

    float height = fragTexCoords.y;

    float sandBlend = smoothstep(0.18, 0.30, height);
    float grassBlend = smoothstep(0.65, 0.75, height);
    float snowBlend = smoothstep(0.80, 0.90, height);

    vec3 terrainColor = mix(sand, grass, sandBlend);
    terrainColor = mix(terrainColor, rock, grassBlend);
    terrainColor = mix(terrainColor, snow, snowBlend);

    float daylight = smoothstep(0.025, 0.15, sunDir.y);

    float diffuse = max(dot(norm, lightDir), 0.0);

    vec3 ambient = terrainColor *
        mix(0.025, 0.2, daylight);

    vec3 directLight = terrainColor *
        diffuse * daylight;

    FragColor = vec4(ambient + directLight, 1.0);
}
