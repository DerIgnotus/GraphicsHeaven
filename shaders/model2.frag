#version 460 core

out vec4 FragColor;

in vec3 Position;
in vec3 Normal;
in vec2 TexCoords;

uniform vec4 baseColor;

uniform float metallic;
uniform float roughness;
uniform vec3 emissive;

uniform sampler2D baseColorTexture;
uniform sampler2D metallicRoughnessTexture;
uniform sampler2D normalTexture;
uniform sampler2D occlusionTexture;
uniform sampler2D emissiveTexture;

uniform bool hasBaseColorTexture;
uniform bool hasMetallicRoughnessTexture;
uniform bool hasNormalTexture;
uniform bool hasOcclusionTexture;
uniform bool hasEmissiveTexture;

void main()
{
    vec4 color = baseColor;

    if (hasBaseColorTexture)
        color *= texture(baseColorTexture, TexCoords);

    float metal = metallic;
    float rough = roughness;

    if (hasMetallicRoughnessTexture)
    {
        vec4 mr = texture(
            metallicRoughnessTexture,
            TexCoords
        );

        rough *= mr.g;
        metal *= mr.b;
    }

    vec3 emission = emissive;

    if (hasEmissiveTexture)
        emission *= texture(
            emissiveTexture,
            TexCoords
        ).rgb;

    FragColor = vec4(
        color.rgb + emission,
        color.a
    );
}