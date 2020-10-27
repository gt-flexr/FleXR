#ifndef __MXRE_GL_CAMERA__
#define __MXRE_GL_CAMERA__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace mxre {
  namespace gl {
    enum camera_movement {
      FORWARD, BACKWARD, LEFT, RIGHT
    };

    const float YAW = 0.0f;
    const float PITCH = 270.0f;
    const float SPEED = 2.5f;
    const float SENSITIVITY = 0.1f;
    const float ZOOM = 45.0f;


    class Camera {
      public:
        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
        glm::vec3 worldUp;

        float yaw, pitch;
        float speed, sensitivity, zoom;

        Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
               float yaw = YAW, float pitch = PITCH, float zoom=ZOOM) : front(glm::vec3(0.0f, 0.0f, -1.0f)),
                speed(SPEED), sensitivity(SENSITIVITY), zoom(zoom) {
          this->position = position;
          this->worldUp = up;
          this->yaw = yaw;
          this->pitch = pitch;
          updateCameraVectors();
        }

        Camera(float pX, float pY, float pZ, float upX, float upY, float upZ, float yaw, float pitch) :
          front(glm::vec3(0.0f, 0.0f, -1.0f)),
          speed(SPEED), sensitivity(SENSITIVITY), zoom(ZOOM)
        {
          this->position = glm::vec3(pX, pY, pZ);
          worldUp = glm::vec3(upX, upY, upZ);
          this->yaw = yaw;
          this->pitch = pitch;
          updateCameraVectors();
        }

        glm::mat4 getViewMatrix() {
          return glm::lookAt(position, position + front, up);
        }


      private:
        void updateCameraVectors() {
          glm::vec3 newFront;

          newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
          newFront.y = sin(glm::radians(pitch));
          newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
          printf("cam xyz: %f %f %f \n", newFront.x, newFront.y, newFront.z);

          front = glm::normalize(newFront);
          right = glm::normalize(glm::cross(front, worldUp));
          up = glm::normalize(glm::cross(right, front));
        }
    };

  } // namespace gl
} // namespace mxre

#endif

