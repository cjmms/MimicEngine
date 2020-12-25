#pragma once

#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/*
* .obj model file
* Architecture of Model:
* Each model contains a tree of nodes
* Each node contains meshes
* Each mesh contain vertices, indices, Material(textures)
*/


class Model
{
public:
	Model(const char* path);
	void Draw(Shader& shader);

private:
	std::vector<Mesh> meshes;

	// path to current model object
	std::string directory;

	// keep track of loaded texture
	// make sure that no redundant texture loading
	std::vector<Texture> textures_loaded;

	// Entry point of loading a model
	void loadModel(std::string path);

	// loading a tree of nodes recursives, for each node, load its mesh
	void processNode(aiNode* node, const aiScene* scene);

	// load vertices, indices and material(textures) from mesh
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Vertex> processVertices(aiMesh* mesh);
	std::vector<unsigned int> processIndices(aiMesh* mesh);
	std::vector<Texture> processTextures(aiMaterial* material);

	// load textures from a material base on texture type
	std::vector<Texture> loadMaterialTextures( aiMaterial* mat, aiTextureType type, 
											   std::string typeName);

	// helper function, convert aiVector3D to glm::vec3
	void setVec3(glm::vec3& des, aiVector3D& source);
};

// helper function
unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

