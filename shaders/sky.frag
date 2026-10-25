#version 460 core
out vec4 fragColor;

uniform mat4 invViewProjection;
uniform vec2 screenSize;
uniform vec3 cameraPosition;
uniform vec3 sunDirection;
uniform float time;

// Procedural noise & hash
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 ip = floor(p);
    vec2 u = fract(p);
    u = u * u * (3.0 - 2.0 * u);
    return mix(mix(rand(ip), rand(ip + vec2(1.0, 0.0)), u.x),
               mix(rand(ip + vec2(0.0, 1.0)), rand(ip + vec2(1.0, 1.0)), u.x), u.y);
}

// Billowy FBM for 3D puffy clouds
float billowFbm(vec2 p) {
    float f = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; i++) {
        f += amp * (1.0 - abs(noise(p)));
        p *= 2.2;
        amp *= 0.5;
    }
    return f;
}

void main() {
    // 1. Reconstruct world-space ray direction
    vec2 ndc = (gl_FragCoord.xy / screenSize) * 2.0 - 1.0;
    vec4 clipNear = vec4(ndc, -1.0, 1.0);
    vec4 clipFar  = vec4(ndc,  1.0, 1.0);
    vec4 worldNear = invViewProjection * clipNear;
    worldNear /= worldNear.w;
    vec4 worldFar = invViewProjection * clipFar;
    worldFar /= worldFar.w;
    vec3 rayDir = normalize(worldFar.xyz - worldNear.xyz);

    vec3 sunDir = normalize(sunDirection);
    float sunHeight = sunDir.y;
    float viewHeight = rayDir.y;

    // 2. Rich Day / Night Color Palette
    vec3 dayZenith   = vec3(0.15, 0.42, 0.85);
    vec3 dayHorizon  = vec3(0.60, 0.78, 0.95);
    vec3 nightZenith = vec3(0.01, 0.01, 0.03);
    vec3 nightHorizon= vec3(0.03, 0.03, 0.08);
    vec3 sunsetColor = vec3(1.0, 0.45, 0.15);

    float sunFactor = clamp(sunHeight * 0.5 + 0.5, 0.0, 1.0);
    vec3 zenithColor = mix(nightZenith, dayZenith, sunFactor);
    vec3 horizonColor = mix(nightHorizon, dayHorizon, sunFactor);

    float altitude = pow(max(0.0, viewHeight), 0.5);
    vec3 skyColor = mix(horizonColor, zenithColor, altitude);

    // Sunset glow
    float sunAlign = dot(rayDir, sunDir);
    float sunsetGlow = max(0.0, sunAlign) * max(0.0, 1.0 - abs(sunHeight) * 2.0);
    skyColor += sunsetColor * sunsetGlow * sunsetGlow * 3.0;

    // Horizon haze
    float haze = pow(1.0 - abs(viewHeight), 3.0) * max(0.0, sunHeight);
    skyColor += horizonColor * haze * 0.4;

    // 3. Sun & Moon
    float sunDist = distance(rayDir, sunDir);
    float sunDisk = smoothstep(0.025, 0.02, sunDist);
    float sunCorona = pow(max(0.0, 1.0 - sunDist * 3.0), 4.0);
    vec3 sunColor = vec3(1.0, 0.95, 0.8) * sunDisk * 20.0;
    sunColor += vec3(1.0, 0.6, 0.2) * sunCorona * max(0.0, sunHeight + 0.1) * 4.0;
    skyColor += sunColor;

    vec3 moonDir = -sunDir;
    float moonDist = distance(rayDir, moonDir);
    float moonDisk = smoothstep(0.02, 0.018, moonDist);
    vec3 moonColor = vec3(0.8, 0.85, 0.9) * moonDisk * 3.0;
    float moonFade = clamp((0.1 - sunHeight) / 0.2, 0.0, 1.0);
    skyColor += moonColor * moonFade;

    // 4. Natural Jittered Night Stars (No Grid Lines, Varied Sizes & Brightness)
    if (viewHeight > 0.0 && sunHeight < 0.1) {
        float nightFade = clamp((0.1 - sunHeight) / 0.3, 0.0, 1.0);
        vec2 starCoord = rayDir.xz / (viewHeight + 0.15) * 450.0;
        vec2 cellID = floor(starCoord);
        vec2 cellUV = fract(starCoord);
        
        float cellRand = rand(cellID);
        if (cellRand > 0.88) { // Sparser distribution
            // Jitter star position inside cell so it's not locked to a grid center
            vec2 starPos = vec2(rand(cellID + 1.3), rand(cellID + 4.7));
            float dist = distance(cellUV, starPos);
            
            // Varied star sizes and brightnesses per star
            float starRadius = 0.04 + 0.12 * rand(cellID + 2.2);
            float starBrightness = rand(cellID + 8.1);
            
            float star = smoothstep(starRadius, 0.0, dist);
            float twinkle = sin(time * 3.0 + cellRand * 50.0) * 0.5 + 0.5;
            
            skyColor += vec3(star * twinkle * nightFade * starBrightness * 2.0);
        }
    }

    // 5. Dynamic Weather & Translating Puffy Clouds (X/Z World Drift)
    if (viewHeight > 0.0) {
        float cloudHeight = 1200.0;
        float safeRayY = max(viewHeight, 0.02);
        float t = (cloudHeight - cameraPosition.y) / safeRayY;
        vec3 worldSamplePos = cameraPosition + rayDir * t;
        
        // --- Continuous X/Z World-Space Wind Drift ---
        // Adding windOffset directly to worldSamplePos.xz physically scrolls the clouds across the X and Z axes
        vec2 windOffset = vec2(time * 2500.0, time * 1200.0) * 5;
        
        vec2 uv1 = (worldSamplePos.xz + windOffset) * 0.00055;
        vec2 uv2 = (worldSamplePos.xz - windOffset * 0.5) * 0.00110;
        
        float baseLayer = billowFbm(uv1);
        float detailLayer = noise(uv2);
        float combinedNoise = mix(baseLayer, detailLayer, 0.35);
        
        // Weather cycle (appearing and disappearing over time)
        float weatherCycle = sin(time * 0.2) * 0.5 + 0.5; 
        float thresholdMin = mix(0.78, 0.38, weatherCycle); 
        float thresholdMax = thresholdMin + 0.25;
        
        float cloudDensity = smoothstep(thresholdMin, thresholdMax, combinedNoise);
        float horizonFade = smoothstep(0.0, 0.15, viewHeight);
        
        vec3 cloudBaseColor = mix(vec3(0.3, 0.33, 0.4), vec3(0.98, 0.98, 1.0), sunFactor);
        cloudBaseColor += sunsetColor * sunsetGlow * 1.5;

        skyColor = mix(skyColor, cloudBaseColor, cloudDensity * horizonFade * 0.95);
    }

    // 6. HDR Tone Mapping & Gamma Correction
    skyColor = skyColor / (skyColor + vec3(1.0));
    skyColor = pow(skyColor, vec3(1.0 / 2.2));

    fragColor = vec4(skyColor, 1.0);
}