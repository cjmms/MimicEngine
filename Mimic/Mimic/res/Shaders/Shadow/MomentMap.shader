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
	float z = FragDepth * 2.0f - 1.0f;

	vec4 b = vec4(z, z*z, z*z*z, z*z*z*z);

	// quantization

	// blur

	// undo quantization

	// invalidate rounding errors

    // compute intensity
   // float intensity = compute_msm_shadow_intensity(b * 0.98f, FragDepth);

	//FragColor = vec4(vec3(intensity), 1.0);
    FragColor = b;
    //FragColor = vec4(1.0f);

}