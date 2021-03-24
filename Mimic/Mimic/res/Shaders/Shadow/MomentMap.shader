#shader vertex
#version 330 core
layout(Location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out float VertexDepth;

// This shader is used to fill depth buffer 
// mvp is from the perspective of light source
// This shader should have no output

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	VertexDepth = vec4(view * model * vec4(aPos, 1.0f)).z;
}



//  This shader is designed to show color buffer

#shader fragment
#version 330 core

in float VertexDepth;
out vec4 FragColor;

vec4 Quan(vec4 b)
{
	b += vec4(0.5, 0.0, 0.5, 0.0);

	// quantization
	vec4 result;

	result.r = dot(vec4(1.5, 0.0, -2.0, 0.0), b);
	result.g = dot(vec4(0.0, 4.0, 0.0, -4.0), b);
	result.b = dot(vec4(sqrt(3) / 2, 0, -sqrt(12) / 9, 0), b);
	result.a = dot(vec4(0, 0.5, 0, 0.5), b);

	return result;
}



void main()
{
    float depth = VertexDepth;

	float dx = dFdx(depth);
	float dy = dFdy(depth);
	float moment2 = depth * depth + 0.25 * (dx * dx + dy * dy);

	// VSM
	FragColor = vec4(depth, moment2, 0.0, 1.0);

	// MSM
	//FragColor = vec4(depth, depth * depth, depth * depth * depth, depth * depth * depth * depth);
}