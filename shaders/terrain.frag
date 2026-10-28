
#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 fragTexCoords;

in float Continentalness;
in float Temperature;
in float Moisture;

out vec4 FragColor;

uniform vec3 sunDir;

uniform sampler2D plainsTexture;
uniform sampler2D desertTexture;
uniform sampler2D forestTexture;
uniform sampler2D hillsTexture;
uniform sampler2D mountainsTexture;
uniform sampler2D snowTexture;

uniform float textureScale = 0.05;
uniform float waterLevel;

/*
vec3 GetBiomeColor()
{
    if (Continentalness < 0.30)
        return vec3(0.08, 0.40, 0.75); // Ocean - blue

    if (Temperature < 0.20)
        return vec3(0.92, 0.95, 0.98); // Snow - white

    if (Temperature > 0.65 && Moisture < 0.30)
        return vec3(0.86, 0.79, 0.56); // Desert - sand

    if (Moisture > 0.60)
        return vec3(0.22, 0.48, 0.24); // Forest - dark green

    if (Continentalness > 0.65)
        return vec3(0.52, 0.52, 0.52); // Mountains - grey

    return vec3(0.38, 0.68, 0.28); // Plains - green
}
*/

float smoothRange(float value, float edge0, float edge1)
{
    return smoothstep(edge0, edge1, value);
}


void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(sunDir);

    vec2 uv = FragPos.xz * textureScale;

    vec3 plains    = texture(plainsTexture, uv).rgb;
    vec3 desert    = texture(desertTexture, uv).rgb;
    vec3 forest    = texture(forestTexture, uv).rgb;
    vec3 hills     = texture(hillsTexture, uv).rgb;
    vec3 mountains = texture(mountainsTexture, uv).rgb;
    vec3 snow      = texture(snowTexture, uv).rgb;

    float desertFactor =
        smoothRange(Temperature, 0.45, 0.75) *
        (1.0 - smoothRange(Moisture, 0.25, 0.60));

    float forestFactor =
        smoothRange(Moisture, 0.35, 0.70) *
        (1.0 - desertFactor);

    float mountainFactor =
        smoothRange(Continentalness, 0.55, 0.85);

    float snowFactor =
        smoothRange(Temperature, 0.15, 0.40) *
        smoothRange(Continentalness, 0.60, 0.85);

    float hillsFactor =
        smoothRange(Continentalness, 0.35, 0.65) *
        (1.0 - mountainFactor);

    vec3 terrainColor = plains;

    terrainColor = mix(terrainColor, desert, desertFactor);
    terrainColor = mix(terrainColor, forest, forestFactor);
    terrainColor = mix(terrainColor, hills, hillsFactor);
    terrainColor = mix(terrainColor, mountains, mountainFactor);
    terrainColor = mix(terrainColor, snow, snowFactor);

    if (Continentalness < 0.30)
    {
        terrainColor = desert;
    }

    float daylight = smoothstep(0.025, 0.15, sunDir.y);
    float diffuse = max(dot(norm, lightDir), 0.0);

    vec3 ambient = terrainColor * mix(0.025, 0.2, daylight);
    vec3 directLight = terrainColor * diffuse * daylight;

    FragColor = vec4(ambient + directLight, 1.0);
}

