#shader vertex
#version 330 core
layout(Location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out float VertexDepth;


void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	VertexDepth = gl_Position.z;
}



//  This shader is designed to show color buffer

#shader fragment
#version 330 core

in float VertexDepth;
out vec4 FragColor;


void main()
{
	float depth = VertexDepth;

	float dx = dFdx(depth);
	float dy = dFdy(depth);
	float moment2 = depth * depth + 0.25 * (dx * dx + dy * dy);

	// VSM
	FragColor = vec4(depth, moment2, 0.0, 1.0);
	//FragColor = vec4(vec3(depth / 30), 1.0);
}