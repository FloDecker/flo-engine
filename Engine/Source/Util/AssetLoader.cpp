//
// Created by flode on 13/03/2023.
//

#include "AssetLoader.h"


Mesh *loadModel(const char *path) {
    auto *importer = new Assimp::Importer();
    const aiScene *scene = importer->ReadFile(path,aiProcess_Triangulate | aiProcess_FlipUVs);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMdP::" << importer->GetErrorString() << std::endl;
        return nullptr;
    }
    Mesh *mesh = new Mesh();
    processNode(scene->mRootNode, scene, mesh);
    return mesh;
}

void processNode(aiNode *node, const aiScene *scene, Mesh *mesh) {
    for (int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh *aimesh = scene->mMeshes[node->mMeshes [i]];
        processMesh(aimesh,scene,mesh);
    }

    for (int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i],scene,mesh);
    }
}

void processMesh(aiMesh *aimesh, const aiScene *scene, Mesh *mesh) {
    auto *vertices = new std::vector<Vertex>;
    Vertex vertex {};

    for (int i = 0; i < aimesh->mNumVertices; ++i) {
        vertex = {};
        vertex.Position.x = aimesh->mVertices[i].x;
        vertex.Position.y = aimesh->mVertices[i].y;
        vertex.Position.z = aimesh->mVertices[i].z;

        vertex.Normal.x = aimesh->mNormals[i].x;
        vertex.Normal.y = aimesh->mNormals[i].y;
        vertex.Normal.z = aimesh->mNormals[i].z;

        vertex.TexCoords.x = aimesh->mTextureCoords[0][i].x;
        vertex.TexCoords.y = aimesh->mTextureCoords[0][i].y;

        (*vertices).push_back(vertex);
    }

    auto *indices = new std::vector<unsigned int>;
    for(unsigned int i = 0; i < aimesh->mNumFaces; i++)
    {
        aiFace face = aimesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            (*indices).push_back(face.mIndices[j]);
    }

    auto *v = new VertexArray(vertices,indices);
    mesh->vertexArrays.push_back(v);
}
