#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}



#shader fragment
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;


struct Material {
    sampler2D texture_albedo;
    sampler2D texture_metallic;
    sampler2D texture_normal;
};

uniform Material material;


void main()
{
    FragColor = texture(material.texture_metallic, TexCoords);
}
