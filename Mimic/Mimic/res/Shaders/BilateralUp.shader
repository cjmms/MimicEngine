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

uniform sampler2D volumetricLightTexture;
uniform sampler2D gPosition;

uniform int BilateralSwitch;

void main()
{

	float upSampledDepth = texture(gPosition, TextureCoord).a;

	vec3 color = vec3(0.0f);
	float totalWeight = 0.0f;

	// Select the closest downscaled pixels.
	int xOffset = mod(TextureCoord.x, 2) == 0 ? -1 : 1;
	int yOffset = mod(TextureCoord.y, 2) == 0 ? -1 : 1;

	vec2 offsets[4];
	offsets[0] = vec2(0.0);
	offsets[1] = vec2(0, yOffset);
	offsets[2] = vec2(xOffset, 0);
	offsets[3] = vec2(xOffset, yOffset);

	
	for (int i = 0; i < 4; i++)
	{
		vec3 downscaledColor = texture(volumetricLightTexture, TextureCoord + offsets[i]).xyz;

		float downscaledDepth = texture(gPosition, TextureCoord + offsets[i]).a;

		float currentWeight = 1.0f;
		currentWeight *= max(0.0f, 1.0f - (0.05f) * abs(downscaledDepth - upSampledDepth));

		color += downscaledColor * currentWeight;
		totalWeight += currentWeight;
	}
	
	vec3 volumetricLight;
	const float epsilon = 0.0001f;
	volumetricLight = color / (totalWeight + epsilon);

	FragColor = vec4(volumetricLight, 1.0f);
	
	// no bilateral upsampling
	if (BilateralSwitch == 0) FragColor = texture(volumetricLightTexture, TextureCoord);
	//if (BilateralSwitch == 0) FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}