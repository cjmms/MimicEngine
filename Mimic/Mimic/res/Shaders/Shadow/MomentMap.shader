#shader vertex
#version 330 core
layout(Location = 0) in vec2 aPos;
layout(Location = 1) in vec2 aTextureCoord;

out vec2 TextureCoord;

void main() {
	gl_Position = vec4(aPos.x, aPos.y, 0.0f, 1.0f);
	TextureCoord = aTextureCoord;
}




//  This shader is designed to show color buffer

#shader fragment
#version 330 core

in vec2 TextureCoord;
out vec4 FragColor;

uniform sampler2D map;


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
    float z = texture(map, TextureCoord).r;

	vec4 b = vec4(z, z*z, z*z*z, z*z*z*z);

	//b = Quan(b);

    FragColor = b;
}