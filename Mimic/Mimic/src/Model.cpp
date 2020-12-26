#include "Model.h"
#include <iostream>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Model::Model(const char* path, bool loadMat )
{
    loadModel(path, loadMat);
}

// iterate all meshes, draw all meshes
void Model::Draw(Shader& shader)
{
	for (auto mesh : meshes) mesh.Draw(shader);
}




void Model::loadModel(std::string path, bool loadMat)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    std::cout << "Loading model, model address: " << path << std::endl;
    double before = glfwGetTime();

    if (loadMat) loadMaterial();
    processNode(scene->mRootNode, scene);
    std::cout << "Model loading complete, loading time: " <<  glfwGetTime() - before << "s " << std::endl;
}


void Model::loadMaterial()
{
    std::cout << "Loading Material" << std::endl;
   loadTexture("albedo.jpg", "texture_albedo");
   loadTexture("metallic.jpg", "texture_metallic");
   loadTexture("normal.jpg", "texture_normal");
   loadTexture("roughness.jpg", "texture_roughness");
   loadTexture("ao.jpg", "texture_ao");
}


void Model::loadTexture(std::string name, std::string type)
{
    Texture texture;
    texture.id = TextureFromFile(name.c_str(), this->directory);
    texture.type = type;
    texture.path = name.c_str();
    textures.push_back(texture);

    std::cout << "Texture Loading: " << name << (texture.id == -1 ? " failed!!!" : " succeed.") << std::endl;
}



void Model::processNode(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}


void Model::setVec3(glm::vec3& des, aiVector3D& source)
{
    des.x = source.x;
    des.y = source.y;
    des.z = source.z;
}


std::vector<Vertex> Model::processVertices(aiMesh* mesh)
{
    std::vector<Vertex> vertices;
    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        // positions
        setVec3(vertex.Position, mesh->mVertices[i]);

        // normals
        if (mesh->HasNormals())
            setVec3(vertex.Normal, mesh->mNormals[i]);

        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoord = vec;

            // tangent
            //setVec3(vertex.Tangent, mesh->mTangents[i]);

            // bitangent
            //setVec3(vertex.Bitangent, mesh->mBitangents[i]);
        }
        else
            vertex.TexCoord = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    return vertices;
}


std::vector<unsigned int> Model::processIndices(aiMesh* mesh)
{
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    return indices;
}




Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices = processVertices(mesh);
    std::vector<unsigned int> indices = processIndices(mesh);

    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, textures, indices);
}




unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        textureID = -1;
        stbi_image_free(data);
    }

    return textureID;
}