#shader vertex
#version 330 core
layout(Location = 0) in vec2 aPos;
layout(Location = 1) in vec2 aTextureCoord;

out vec2 TexCoords;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0f, 1.0f);
    TexCoords = aTextureCoord;
}







#shader fragment
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 camPos;

uniform sampler2D volumetricLightTexture;

uniform sampler2D gPosition;
uniform sampler2D gAlbedoMetallic;
uniform sampler2D gNormalRoughness;

const float PI = 3.141f;

const int N_LIGHTS = 10;
uniform vec3 lightPositions[N_LIGHTS];
uniform vec3 lightColors[N_LIGHTS];

uniform mat4 lightProjection;
uniform mat4 lightView;
uniform sampler2D shadowMap;




// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
// ----------------------------------------------------------------------------


// reflection equation
vec3 reflection(vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, vec3 F0, vec3 WorldPos)
{
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < N_LIGHTS; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }
    return Lo;
}

/// Computes the Moment-shadow intensity for the fragment depth
/// 
/// - b is RGBA value that you got from sampling the texture and 
///   putting it through the reverse matrix.
/// - fragment_depth is the sample point at which you want to calculate
///   the shadow
/// - The return value is the shadow value
float compute_msm_shadow_intensity(vec4 b, float fragment_depth) {

    // OpenGL 4 only - fma has higher precision:
    // float l32_d22 = fma(-b.x, b.y, b.z); // a * b + c
    // float d22 = fma(-b.x, b.x, b.y);     // a * b + c
    // float squared_depth_variance = fma(-b.x, b.y, b.z); // a * b + c

    float l32_d22 = -b.x * b.y + b.z;
    float d22 = -b.x * b.x + b.y;
    float squared_depth_variance = -b.x * b.y + b.z;

    float d33_d22 = dot(vec2(squared_depth_variance, -l32_d22), vec2(d22, l32_d22));
    float inv_d22 = 1.0 - d22;
    float l32 = l32_d22 * inv_d22;

    float z_zero = fragment_depth;
    vec3 c = vec3(1.0, z_zero - b.x, z_zero * z_zero);
    c.z -= b.y + l32 * c.y;
    c.y *= inv_d22;
    c.z *= d22 / d33_d22;
    c.y -= l32 * c.z;
    c.x -= dot(c.yz, b.xy);

    float inv_c2 = 1.0 / c.z;
    float p = c.y * inv_c2;
    float q = c.x * inv_c2;
    float r = sqrt((p * p * 0.25) - q);

    float z_one = -p * 0.5 - r;
    float z_two = -p * 0.5 + r;

    vec4 switch_msm;
    if (z_two < z_zero) {
        switch_msm = vec4(z_one, z_zero, 1.0, 1.0);
    }
    else {
        if (z_one < z_zero) {
            switch_msm = vec4(z_zero, z_one, 0.0, 1.0);
        }
        else {
            switch_msm = vec4(0.0);
        }
    }

    float quotient = (switch_msm.x * z_two - b.x * (switch_msm.x + z_two + b.y)) / ((z_two - switch_msm.y) * (z_zero - z_one));
    return clamp(switch_msm.y + switch_msm.z * quotient, 0.0, 1.0);
}



float calculateShadowIntensity(vec3 N, vec3 WorldPos)
{
    vec4 lightSpaceFragPos = lightProjection * lightView * vec4(WorldPos, 1.0f);
    vec3 projCoord = lightSpaceFragPos.xyz / lightSpaceFragPos.w;
    float FragDepth = projCoord.z;
    // transform to [0,1] range
    projCoord = projCoord * 0.5 + 0.5;

    if (projCoord.z > 1 || projCoord.z < 0) return 0;
    if (projCoord.x > 1 || projCoord.x < 0) return 0;
    if (projCoord.y > 1 || projCoord.y < 0) return 0;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    vec4 b =  texture(shadowMap, projCoord.xy);

    float intensity = compute_msm_shadow_intensity(  b, FragDepth);

    float shadow = FragDepth > (b.x ) ? 0.01 : 1.0;

    //if (FragDepth > intensity) return intensity;
    //else return 0.0;

    //float shadow = FragDepth > (intensity * 0.5 + 0.5) ? 0.01 : 1.0;

    return shadow;
    //return compute_msm_shadow_intensity(0.98 * b, FragDepth);
}



//----------------------------------------------------------------------
float calculateShadow(vec3 N, vec3 WorldPos)
{
    vec4 lightSpaceFragPos = lightProjection * lightView * vec4(WorldPos, 1.0f);
    vec3 projCoord = lightSpaceFragPos.xyz / lightSpaceFragPos.w;
    // transform to [0,1] range
    projCoord = projCoord * 0.5 + 0.5;

    if (projCoord.z > 1 || projCoord.z < 0) return 0;
    if (projCoord.x > 1 || projCoord.x < 0) return 0;
    if (projCoord.y > 1 || projCoord.y < 0) return 0;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoord.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoord.z;

    float bias = 0.00078f;
    // check whether current frag pos is in shadow
    float shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;

    //if (closestDepth > 0.0f) return 1.0;
    return shadow;
}



void main()
{
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;

    vec3 albedo = pow(texture(gAlbedoMetallic, TexCoords).rgb, vec3(2.2));
    float metallic = texture(gAlbedoMetallic, TexCoords).a;
    float roughness = texture(gNormalRoughness, TexCoords).a;
    // from 0 to 1
    float depth = texture(gPosition, TexCoords).a;

    vec3 N = texture(gNormalRoughness, TexCoords).rgb;
    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = mix(vec3(0.04f), albedo, metallic);

    // reflectance equation
    vec3 Lo = reflection(N, V, albedo, metallic, roughness, F0, WorldPos);

    // NO AO
    vec3 color = Lo;


    // volumetric lighting
    //color += 0.01f * texture(volumetricLightTexture, TexCoords).xyz;

    // shadow
    //color *= 1 - calculateShadow(N, WorldPos);
    //float shadowIntensity = calculateShadowIntensity(N, WorldPos);
    //if (shadowIntensity != 0) color *= shadowIntensity;


    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    // Debuging purposes
    FragColor = vec4(albedo, 1.0f);
    FragColor = vec4(N, 1.0f);
    FragColor = vec4(WorldPos, 1.0f);
    //FragColor = vec4(depth, depth, depth, 1.0f);
    FragColor = vec4(vec3(metallic), 1.0f);
    FragColor = vec4(vec3(roughness), 1.0f);


    FragColor = vec4(color, 1.0f);
}
