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

// --- Lighting & Scene Uniforms ---
uniform vec3 sunDirection;  // Driven by your procedural sky
uniform vec3 cameraPos;     // From camera.GetPosition()

const float PI = 3.14159265359;

// PBR Helper Functions
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return a2 / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / max(denom, 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // 1. Base Color / Albedo
    vec4 texColor = baseColor;
    if (hasBaseColorTexture)
        texColor *= texture(baseColorTexture, TexCoords);
    
    // Discard transparent pixels if needed (alpha testing)
    if (texColor.a < 0.1)
        discard;

    vec3 albedo = texColor.rgb;

    // 2. Metallic & Roughness
    float metal = metallic;
    float rough = roughness;

    if (hasMetallicRoughnessTexture)
    {
        vec4 mr = texture(metallicRoughnessTexture, TexCoords);
        rough *= mr.g; // G channel is typically roughness in glTF
        metal *= mr.b; // B channel is typically metallic in glTF
    }
    rough = clamp(rough, 0.04, 1.0); // Prevent division by zero

    // 3. Ambient Occlusion
    float ao = 1.0;
    if (hasOcclusionTexture)
        ao = texture(occlusionTexture, TexCoords).r;

    // 4. Emissive
    vec3 emission = emissive;
    if (hasEmissiveTexture)
        emission *= texture(emissiveTexture, TexCoords).rgb;

    // 5. Normal Vector
    vec3 N = normalize(Normal);
    // Note: If you implement TBN normal mapping later, modify N here using normalTexture.

    // 6. Lighting Vectors
    vec3 V = normalize(cameraPos - Position);
    vec3 L = normalize(sunDirection); // Sun light direction
    vec3 H = normalize(V + L);

    // Dynamic Sun Color (Bright white-yellow during day, warm amber/dim at sunset/night)
    float sunHeight = sunDirection.y;
    vec3 sunColor = vec3(1.0, 0.95, 0.85) * max(sunHeight * 2.0, 0.05);

    // Calculate reflectance at normal incidence; if dia-electric use base F0 of 0.04
    // and if metal, use albedo as F0 weighted by metallic value
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metal);

    // --- Cook-Torrance BRDF ---
    float NDF = DistributionGGX(N, H, rough);
    float G   = GeometrySmith(N, V, L, rough);
    vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - metallic);

    float NdotL = max(dot(N, L), 0.0);

    // Direct lighting contribution from the sun
    vec3 radiance = sunColor;
    vec3 directLighting = (kD * albedo / PI + specular) * radiance * NdotL;

    // Ambient lighting term (soft sky ambient so shadowed areas aren't pitch black)
    vec3 ambient = vec3(0.08, 0.1, 0.15) * albedo * ao;

    // Final color combination
    vec3 color = ambient + directLighting + emission;

    // HDR Tone mapping & Gamma correction (matches your sky shader)
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, texColor.a);
}