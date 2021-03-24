#shader vertex
#version 330 core

layout(Location = 0) in vec3 aPos;
layout(Location = 1) in vec3 aNormal;
layout(Location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


out VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoord;
    vec4 FragPosLightSpace;
} vs_out;


uniform mat4 lightProjection;
uniform mat4 lightView;


void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);

	vs_out.FragPos = vec3(model * vec4(aPos, 1.0f));
	vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
	vs_out.TexCoord = aTexCoord;
    vs_out.FragPosLightSpace = lightProjection * lightView * model * vec4(aPos, 1.0f);

}





#shader fragment
#version 330 core


in VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoord;
    vec4 FragPosLightSpace;
} fs_in;


const int N_LIGHTS = 10;
uniform vec3 lightPositions[N_LIGHTS];
uniform vec3 lightColors[N_LIGHTS];
uniform vec3 camPos;


uniform sampler2D ShadowMap;
uniform sampler2D MSM;
uniform sampler2D VSM;
//uniform mat4 lightProjection;
//uniform mat4 lightView;

out vec4 FragColor;


vec3 calculateLighting(vec3 normal, int i, vec3 color)
{
	// diffuse
	vec3 lightDir = normalize(lightPositions[i] - fs_in.FragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 result = lightColors[i] * diff * color;
	// attenuation (use quadratic as we have gamma correction)
	float distance = length(fs_in.FragPos - lightPositions[i]);
	result *= 1.0 / (distance * distance);
	return result;
}





vec3 solveCramers(vec4 bPrime, vec3 result)
{
    mat3 m0 =
        mat3(1, bPrime.x, bPrime.y,
            bPrime.x, bPrime.y, bPrime.z,
            bPrime.y, bPrime.z, bPrime.w);
    mat3 m1 =
        mat3(result.x, bPrime.x, bPrime.y,
            result.y, bPrime.y, bPrime.z,
            result.z, bPrime.z, bPrime.w);
    mat3 m2 =
        mat3(1, result.x, bPrime.y,
            bPrime.x, result.y, bPrime.z,
            bPrime.y, result.z, bPrime.w);
    mat3 m3 =
        mat3(1, bPrime.x, result.x,
            bPrime.x, bPrime.y, result.y,
            bPrime.y, bPrime.z, result.z);

    float det0 = determinant(m0);
    float det1 = determinant(m1);
    float det2 = determinant(m2);
    float det3 = determinant(m3);

    float x = det1 / det0;
    float y = det2 / det0;
    float z = det3 / det0;

    return vec3(x, y, z);
}


float momentShadowFactor(vec4 shadowSample, float depth)
{
    vec4 bPrime = shadowSample;
    vec3 depthVec = vec3(1.f, depth, depth * depth);

    //-- Calc C. decomp = M, depthVec = D
    vec3 c = solveCramers(bPrime, depthVec);

    //-- Calc roots
    //solve c3 * z^2 + c2 * z + c1 = 0 for z using quadratic formula 
    // -c2 +- sqrt(c2^2 - 4 * c3 * c1) / (2 * c3)
    //Let z2 <= z3 denote solutions 
    float sqrtTerm = sqrt(c.y * c.y - 4 * c.z * c.x);
    float z2 = (-c.y - sqrtTerm) / (2 * c.z);
    float z3 = (-c.y + sqrtTerm) / (2 * c.z);

    if (z2 > z3)
    {
        float temp = z2;
        z2 = z3;
        z3 = temp;
    }

    //-- If depth <= z2
    if (depth <= z2) return 0;

    //-- If depth <= z3
    //  g = (depth * z3 - b'1(depth + z3) + b'2) / ((z3 - z2) * (depth - z2))
    //  return g
    if (depth <= z3)
    {
        return (depth * z3 - bPrime.x * (depth + z3) + bPrime.y) / ((z3 - z2) * (depth - z2));
    }
    //-- if depth > z3
    //  g = (z2 * z3 - b'1(z2 + z3) + b'2) / ((depth - z2) * (depth - z3))
    //  return 1 - g
    else
    {
        return 1 - (z2 * z3 - bPrime.x * (z2 + z3) + bPrime.y) / ((depth - z2) * (depth - z3));
    }
}




float MSM_Intensity(vec4 b, float fragment_depth)
{
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




vec4 UndoQuantization(vec4 b)
{
    b -= vec4(0.5, 0.0, 0.5, 0.0);

    vec4 result;

    result.r = dot(vec4(-(1 / 3), 0.0, sqrt(3), 0.0), b);
    result.g = dot(vec4(0.0, 0.125, 0.0, 1.0), b);
    result.b = dot(vec4(-0.75, 0.0, 0.75 * sqrt(3), 0.0), b);
    result.a = dot(vec4(0.0, -0.125, 0.0, 1.0), b);

    return result;
}

vec4 Invalidate(vec4 b)
{
    float alpha = 1 * pow(10, -5);
    return (1.0f - alpha) * b + alpha * 0.5;
}



float calculateShadow(vec4 fragPosLightSpace)
{
    vec3 projCoord = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoord = projCoord * 0.5 + 0.5;

    if (projCoord.z > 1 || projCoord.z < 0) return 1;
    if (projCoord.x > 1 || projCoord.x < 0) return 1;
    if (projCoord.y > 1 || projCoord.y < 0) return 1;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(ShadowMap, projCoord.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoord.z;


    //float closestDepth = texture(VSM, projCoord.xy).r;
    // get depth of current fragment from light's perspective
    //float currentDepth = fragPosLightSpace.z;



    float bias = 0.000078f;
    // check whether current frag pos is in shadow
    return (currentDepth - bias) > closestDepth ? 0.0 : 1.0;
}



float calculateMSM(vec4 fragPosLightSpace)
{
    vec3 projCoord = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoord = projCoord * 0.5 + 0.5;

    if (projCoord.z > 1 || projCoord.z < 0) return 1;
    if (projCoord.x > 1 || projCoord.x < 0) return 1;
    if (projCoord.y > 1 || projCoord.y < 0) return 1;

    vec4 b = texture(MSM, projCoord.xy);
    //vec4 b = vec4(texture(ShadowMap, projCoord.xy).x);
    //b = UndoQuantization(b);

    return 1 - MSM_Intensity(Invalidate(b), fragPosLightSpace.w);
}


float linstep(const float low, const float high, const float value) {
    return clamp((value - low) / (high - low), 0.0, 1.0);
}

// ----------------------------------------------------------------------------
// Variance shadow mapping
float calculateVSM(vec4 fragPosLightSpace) {
    // Perspective divide
    vec2 screenCoords = fragPosLightSpace.xy / fragPosLightSpace.w;
    screenCoords = screenCoords * 0.5 + 0.5; // [0, 1]

     float distance = fragPosLightSpace.z; // Use raw distance instead of linear junk
     vec2 moments = texture2D(VSM, screenCoords.xy).rg;

     float p = step(distance, moments.x);
     float variance = max(moments.y - (moments.x * moments.x), 0.00002);
     float d = distance - moments.x;
     float pMax = linstep(0.2, 1.0, variance / (variance + d * d)); // Solve light bleeding

    return min(max(p, pMax), 1.0);
}




void main()
{
    //float shadowIntensity = calculateShadow(fs_in.FragPosLightSpace);

    //float shadowIntensity = calculateMSM(fs_in.FragPosLightSpace);

    float shadowIntensity = calculateVSM(fs_in.FragPosLightSpace);


	vec3 color = vec3(1.0f);
	vec3 normal = normalize(fs_in.Normal);

	vec3 ambient = 0.05 * color;
	vec3 diffuse = vec3(0.0f);

	for (int i = 0; i < 4; ++i)
	{
		diffuse += calculateLighting(normal, i, color);
	}


	FragColor = vec4((ambient + diffuse) * shadowIntensity, 1.0f);


    vec2 screenCoords = fs_in.FragPosLightSpace.xy / fs_in.FragPosLightSpace.w;
    screenCoords = screenCoords * 0.5 + 0.5; // [0, 1]

    float distance = fs_in.FragPosLightSpace.z; // Use raw distance instead of linear junk
    vec2 moments = texture2D(VSM, screenCoords.xy).rg;

    FragColor = vec4(vec3(moments.x / 30), 1.0);
    //FragColor = vec4(vec3(distance / 30), 1.0);
    //FragColor = vec4(fs_in.FragPosLightSpace.xyz, 1.0f);

    //FragColor = vec4(vec3(texture(ShadowMap, screenCoords).r) / 2, 1.0f);

    //vec4(FragPos,1.0)
}