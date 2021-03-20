#shader vertex
#version 330 core

layout(Location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// This shader is used to fill depth buffer 
// mvp is from the perspective of light source
// This shader should have no output

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}




#shader fragment
#version 330 core

out vec4 FragColor;


void main()
{
	float z = gl_FragCoord.z;
	float z2 = z * gl_FragCoord.z;
	float z3 = z2 * gl_FragCoord.z;
	float z4 = z3 * gl_FragCoord.z;

	FragColor = vec4(z, z2, z3, z4);
	//FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
}