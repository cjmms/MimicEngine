#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPos = (model * vec4(aPos, 1.0)).xyz;
}



#shader fragment
#version 330 core

in vec3 FragPos;
out vec4 FragColor;

uniform sampler2D ShadowMap;
uniform mat4 lightProjection;
uniform mat4 lightView;


float calculateShadow(vec3 WorldPos)
{
	vec4 lightSpaceFragPos = lightProjection * lightView * vec4(WorldPos, 1.0f);
	vec3 projCoord = lightSpaceFragPos.xyz / lightSpaceFragPos.w;
	// transform to [0,1] range
	projCoord = projCoord * 0.5 + 0.5;

	if (projCoord.z > 1 || projCoord.z < 0) return 0;
	if (projCoord.x > 1 || projCoord.x < 0) return 0;
	if (projCoord.y > 1 || projCoord.y < 0) return 0;

	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(ShadowMap, projCoord.xy).r;
	// get depth of current fragment from light's perspective
	float currentDepth = projCoord.z;


	// check whether current frag pos is in shadow
	float shadow = currentDepth  > closestDepth ? 1.0 : 0.0;

	//if (closestDepth > 0.0f) return 1.0;
	return shadow;
}


void main()
{

	float shadowIntensity = 1 - calculateShadow(FragPos);

	vec3 lighting = vec3(1.0f);

	lighting *= shadowIntensity;

	FragColor = vec4(lighting, 1.0f);
}