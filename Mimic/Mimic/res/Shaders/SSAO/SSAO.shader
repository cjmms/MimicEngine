#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}





#shader fragment
#version 330 core

// only need the red channel
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise; // noise texture(random rotation)

uniform int kernelSize;     // number of random sampled positions in hemisphere
uniform float radius;       // radius if hemisphere

uniform vec3 samples[64];   // random sample kernel
uniform mat4 view;
uniform mat4 projection;

uniform float screenWidth;
uniform float screenHeight;

uniform float noiseTexLength;



void main()
{
    // view space frag position  
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).xyz);

    // scale the noise texture so that it maps the whole screen
    vec2 noiseScale = vec2(screenWidth / noiseTexLength, screenHeight / noiseTexLength);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    // prepare an orthogonal basis for transformation from tangent space to view space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);    // orthogonal basis


    // compute occlusion
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        vec3 samplePos = TBN * samples[i];      // transform sample from tangent space to view space
        samplePos = fragPos + samplePos * radius;

        // transform samplePos to screen space in order to sample depth value
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * view * offset;           // view space to clip space
        offset.xyz /= offset.w;                 // perspective division
        offset.xyz = offset.xyz * 0.5 + 0.5;    // mapping from [-1, +1] to [0, 1]

        // access sample depth
        float sampleDepth = texture(gPosition, offset.xy).z;

        float bias = 0.015;
        
        //float range = 1 - radius / abs(fragPos.z - sampleDepth);
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0);
    }


    // normalize occlusion by size of kernel
    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = occlusion;
}



