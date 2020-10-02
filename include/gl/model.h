#ifndef __MXRE_GL_MODEL__
#define __MXRE_GL_MODEL__

#include <bits/stdc++.h>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb/stb_image.h>
#include "defs.h"
#include "mesh.h"
#include "shader.h"

using namespace std;

namespace mxre {
  namespace gl {
    inline unsigned int readTextureFromFile(const char *path, const string directory, bool gamma = false);

    class Model {
    public:
      vector<Texture> loadedTextures;
      vector<Mesh> meshes;
      string directory;
      bool gammaCorrection;

      Model(string const &path, bool gamma=false) : gammaCorrection(gamma) {
        loadModel(path);
      }

      void draw(Shader &shader) {
        // 1. Draw all the meshes of this model
        for(unsigned int i = 0; i < meshes.size(); i++)
          meshes[i].draw(shader);
      }


    private:
      void loadModel(string const &path) {
        Assimp::Importer importer;

        // 1. Read the 3D object into Assimp's scene
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
          debug_print("%s", importer.GetErrorString());
          return;
        }

        // 2. Recursively process Assimp's root node
        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
      }


      void processNode(aiNode *node, const aiScene *scene) {
        // 1. Process and store all the meshes of a node w.r.t the scene
        for(unsigned int i = 0; i < node->mNumMeshes; i++) {
          aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
          meshes.push_back(processMesh(mesh, scene));
        }

        // 2. Keep the node-processing recursion
        for(unsigned int i = 0; i < node->mNumChildren; i++) {
          processNode(node->mChildren[i], scene);
        }
      }


      void convertAiVec3ToGlmVec3(glm::vec3 &glmVec, aiVector3D &aiVec) {
        glmVec.x = aiVec.x;
        glmVec.y = aiVec.y;
        glmVec.z = aiVec.z;
      }

      Mesh processMesh(aiMesh* mesh, const aiScene *scene) {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // 1. Convert Assimp vertex into GL vector
        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
          Vertex vertex;
          glm::vec3 placeholder;

          // 1.1. Convert the position vector
          convertAiVec3ToGlmVec3(placeholder, mesh->mVertices[i]);
          vertex.position = placeholder;

          // 1.2. Convert the normal vector
          if(mesh->HasNormals()) {
            convertAiVec3ToGlmVec3(placeholder, mesh->mNormals[i]);
            vertex.normal = placeholder;
          }

          // 1.3. Convert the texture coords, tangents, and bitangents
          if(mesh->mTextureCoords[0]) {
            vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

            convertAiVec3ToGlmVec3(placeholder, mesh->mTangents[i]);
            vertex.tan = placeholder;

            convertAiVec3ToGlmVec3(placeholder, mesh->mBitangents[i]);
            vertex.bitan = placeholder;
          }
          else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
          }

          vertices.push_back(vertex);
        }

        // 2. Convert mesh's faces into the GL vertex indices
        for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
          aiFace face = mesh->mFaces[i];
          for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
        }

        // 3. Convert assimp's materials (textures) into the GL mesh textures
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // 3.1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // 3.2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // 3.3. normal maps
        vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        // 3.4. height maps
        vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        return Mesh(vertices, indices, textures);
      }


      vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
        vector<Texture> textures;

        // 1. Check all material textures and load textures if they are not loaded yet
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
          aiString str;
          mat->GetTexture(type, i, &str);
          bool skip = false;

          // 1.1. Check if the texture was loaded already. If so, skip loading
          for(unsigned int j = 0; j < loadedTextures.size(); j++) {
            if(strcmp(loadedTextures[j].path.data(), str.C_Str()) == 0) {
              textures.push_back(loadedTextures[j]);
              skip = true;
              break;
            }
          }

          // 1.2. Load a new texture
          if(!skip) {
            Texture texture;
            texture.id = readTextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            loadedTextures.push_back(texture);
          }
        }

        return textures;
      }
    };


    inline unsigned int readTextureFromFile(const char *path, const string directory, bool gamma) {
      string filename = string(path);
      filename = directory + '/' + filename;

      unsigned int textureID;
      glGenTextures(1, &textureID);

      int width, height, componentNum;
      unsigned char *data = stbi_load(filename.c_str(), &width, &height, &componentNum, 0);
      if(data) {
        GLenum format;
        if(componentNum == 1)
          format = GL_RED;
        else if(componentNum == 3)
          format = GL_RGB;
        else if(componentNum == 4)
          format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0); // ?    

        stbi_image_free(data);
      }
      else {
        debug_print("Texture failed to load at path %s", path);
        stbi_image_free(data);
      }

      return textureID;
    }
  } // namespace gl
} // namespace mxre
#endif

