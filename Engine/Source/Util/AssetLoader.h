//
// Created by flode on 13/03/2023.
//

#ifndef ENGINE_ASSETLOADER_H
#define ENGINE_ASSETLOADER_H

#include "../Content/Mesh.h"
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

Mesh *loadModel(const char* path);
void processNode(aiNode *node, const aiScene *scene, Mesh *mesh);
void processMesh(aiMesh *aimesh, const aiScene *scene, Mesh *mesh);
#endif //ENGINE_ASSETLOADER_H
