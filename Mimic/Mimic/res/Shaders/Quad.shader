#shader vertex
#version 330 core
layout(Location = 0) in vec2 aPos;
layout(Location = 1) in vec2 aTextureCoord;

out vec2 TextureCoord;

void main() {
	gl_Position = vec4(aPos.x, aPos.y, 0.0f, 1.0f);
	TextureCoord = aTextureCoord;
}




#shader fragment
#version 330 core

in vec2 TextureCoord;
out vec4 FragColor;

uniform sampler2D map;


void main()
{
	// depth buffer test
	//vec3 color = vec3(texture(map, TextureCoord).r);
	//FragColor = vec4(color, 1.0f);

	// color buffer test
	FragColor = texture(map, TextureCoord);
	FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}