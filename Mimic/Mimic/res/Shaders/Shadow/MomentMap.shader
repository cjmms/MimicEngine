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

void main()
{
	// convert z from [0, 1] to [-1, 1]
    float FragDepth = texture(map, TextureCoord).r;
	float z = FragDepth * 2 - 1;

	vec4 b = vec4(z, z*z, z*z*z, z*z*z*z);

	// quantization

	// blur

	// undo quantization

	// invalidate rounding errors
	float alpha = 6 * pow(10, -5);
	b = (1.0f - alpha) * b + alpha * (0.0, 0.63, 0.0, 0.63);

    FragColor = b;
}