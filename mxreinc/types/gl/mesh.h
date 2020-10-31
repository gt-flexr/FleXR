#ifndef __MXRE_GL_MESH__
#define __MXRE_GL_MESH__

#include <GL/glew.h>

#include <string>
#include <vector>

#include "types/gl/shader.h"

using namespace std;

namespace mxre {
  namespace gl_types {
    struct Vertex {
      glm::vec3 position;
      glm::vec3 normal;
      glm::vec2 texCoords;
      glm::vec3 tan;
      glm::vec3 bitan;
    };

    struct Texture {
      unsigned int id;
      string type, path;
    };

    class Mesh {
    private:
      unsigned int vbo, ebo;

      void setupMesh() {
        // 1. Create buffers and vectex array
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        // 2. Bind the vertex array and Load data & indices into vertex buffers
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // 3. Set the vertex attribute pointers
        // 3.1. vertex position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // 3.2. vertex normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        // 3.3. vertex texcoords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

        // 3.4. vertex tan
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tan));

        // 3.4. vertex bitan
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitan));

        // 4. Unbind vertex array
        glBindVertexArray(0);
      }


    public:
      vector<Vertex> vertices;
      vector<unsigned int> indices;
      vector<Texture> textures;
      unsigned int vao;

      Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures):
        vertices(vertices), indices(indices), textures(textures) {
          setupMesh();
      }

      void draw(Shader &shader) {
        unsigned int diffuseNum = 1;
        unsigned int specularNum = 1;
        unsigned int normalNum = 1;
        unsigned int heightNum = 1;

        // 1. Find the active texture and bind
        for(unsigned int i = 0; i < textures.size(); i++) {
          string number;
          string name = textures[i].type;

          // 1.1. Activate the proper texture unit
          glActiveTexture(GL_TEXTURE0 + i);

          // 1.2. Retrieve the texture number
          if(name == "texture_diffuse")
            number = std::to_string(diffuseNum++);
          else if(name == "texture_specular")
            number = std::to_string(specularNum++);
          else if(name == "texture_normal")
            number = std::to_string(normalNum++);
          else if(name == "texture_height")
            number = std::to_string(heightNum++);

          // 1.3. Set the shader's sampler to the corresponding texture unit
          glUniform1i( glGetUniformLocation(shader.ID, (name+number).c_str()), i );

          // 1.4. Bind the texture
          glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // 2. Bind and draw the vertices with the bound textures
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
      }
    };
  } // namespace gl_types
} // namespace mxre

#endif

