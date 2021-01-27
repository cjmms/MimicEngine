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

    float bias = 0.001f;
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
    color += 0.05f * texture(volumetricLightTexture, TexCoords).xyz;

    // shadow
    //color *= 1 - calculateShadow(N, WorldPos);


    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0f);
}
