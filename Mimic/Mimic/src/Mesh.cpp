
#include "Mesh.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"





Mesh::Mesh(std::vector<Vertex> vertices,
	std::vector<Texture> textures,
	std::vector<unsigned int> indices)
	: vertices(vertices), textures(textures), indices(indices)

{
	setup();
}



void Mesh::Draw(Shader& shader)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;

    // setup texture uniform
    // the name of uniform will base on type of texture and number of textures
    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);

        shader.setFloat(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setup()
{
    // VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // VBO
    VertexBuffer vbo(&vertices[0], vertices.size() * sizeof(Vertex));

    // IBO
    IndexBuffer ibo(&indices[0], indices.size());

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord));

    glBindVertexArray(0);
}

