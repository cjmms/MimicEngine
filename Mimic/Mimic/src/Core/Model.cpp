#include "Model.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include "../ResourceManager.h"
#include <assimp/pbrmaterial.h>




Model::Model(const char* path, bool loadMat )
{
    loadModel(path, loadMat);
}

// iterate all meshes, draw all meshes
void Model::Draw(Shader& shader)
{
    shader.Bind();
	for (auto mesh : meshes) mesh.Draw(shader);
    shader.unBind();
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

    //if (loadMat) loadMaterial();
    processNode(scene->mRootNode, scene);
    std::cout << "Model loading complete, loading time: " <<  glfwGetTime() - before << "s " << std::endl;
}
/*

void Model::loadMaterial()
{
    std::cout << "Loading Material" << std::endl;
   loadTexture("albedo.tga", "texture_albedo");
   loadTexture("metallic.jpg", "texture_metallic");
   loadTexture("normal.jpg", "texture_normal");
   loadTexture("roughness.jpg", "texture_roughness");
   loadTexture("ao.jpg", "texture_ao");
}
*/
/*
void Model::loadTexture(std::string name, std::string type)
{
    Texture texture;
    texture.id = ResourceManager::TextureFromFile(name.c_str(), this->directory);
    texture.type = type;
    texture.path = name.c_str();
    textures.push_back(texture);

    std::cout << "Texture Loading: " << name << (texture.id == -1 ? " failed!!!" : " succeed.") << std::endl;
}
*/


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

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN


    std::vector<Texture> textures;
    // 1. diffuse maps
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_albedo");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_metallic");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_roughness");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, textures, indices);
}


std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        //if (type != aiTextureType_UNKNOWN)
            mat->GetTexture(type, i, &str);
       // else
         //   mat->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, i, &str);
        //mat->GetTexture(aiTextureType_UNKNOWN);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = ResourceManager::TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
        }
    }
    return textures;
}