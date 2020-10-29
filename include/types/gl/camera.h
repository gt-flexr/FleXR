#ifndef __MXRE_GL_CAMERA__
#define __MXRE_GL_CAMERA__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <bits/stdc++.h>

namespace mxre {
  namespace gl_types {
    enum camera_movement {
      FORWARD, BACKWARD, LEFT, RIGHT
    };

    const float YAW = 0.0f;
    const float PITCH = 270.0f;
    const float SPEED = 0.1f;
    const float ANGLE = 5.0f;
    const float ZOOM = 45.0f;


    class Camera {
      public:
        glm::vec3 position, initPosition;
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
        glm::vec3 worldUp;

        float yaw, initYaw, pitch, initPitch;
        float speed, angle, zoom;

        Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
               float yaw = YAW, float pitch = PITCH, float zoom=ZOOM, float angle=ANGLE) :
               front(glm::vec3(0.0f, 0.0f, -1.0f)), speed(SPEED), angle(angle), zoom(zoom) {
          this->position = position;
          this->initPosition = position;

          this->worldUp = up;
          this->yaw = yaw;
          this->initYaw = yaw;
          this->pitch = pitch;
          this->initPitch = pitch;
          updateCameraVectors();
        }

        Camera(float pX, float pY, float pZ, float upX, float upY, float upZ, float yaw, float pitch) :
          front(glm::vec3(0.0f, 0.0f, -1.0f)),
          speed(SPEED), angle(ANGLE), zoom(ZOOM)
        {
          this->position = glm::vec3(pX, pY, pZ);
          this->initPosition = glm::vec3(pX, pY, pZ);

          this->worldUp = glm::vec3(upX, upY, upZ);
          this->yaw = yaw;
          this->initYaw = yaw;
          this->pitch = pitch;
          this->initPitch = pitch;
          updateCameraVectors();
        }


        glm::mat4 getViewMatrix() {
          return glm::lookAt(position, position + front, up);
        }


        void resetCamera() {
          this->position = this->initPosition;
          this->front = glm::vec3(0.0f, 0.0f, -1.0f);
          this->yaw = this->initYaw;
          this->pitch = this->initPitch;
          updateCameraVectors();
        }


        void processTranslation(camera_movement direction) {
          if(direction == FORWARD)
            position += front * speed;
          if(direction == BACKWARD)
            position -= front * speed;
          if(direction == LEFT)
            position += right * speed;
          if(direction == RIGHT)
            position -= right * speed;
        }

        void processRotation(camera_movement direction) {
          if(direction == FORWARD)
            pitch += angle;
          if(direction == BACKWARD)
            pitch -= angle;
          if(direction == LEFT)
            yaw += angle;
          if(direction == RIGHT)
            yaw -= angle;

          // preventing pitch reversion
          if (pitch > 359.0f)
            pitch = 359.0f;
          if (pitch < 180.0f)
            pitch = 180.0f;
          printf("Yaw(%f) Pitch(%f) \n", yaw, pitch);
          updateCameraVectors();
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

  } // namespace gl_types
} // namespace mxre

#endif

