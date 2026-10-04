#ifndef SIM_H
#define SIM_H
#include <vector>
#include <cmath>
#include "body.h"
#include <GLFW/glfw3.h>


class Gravity {
  public:
    void applyGravity(std::vector<Body>& bodies, float dt) {
        for (auto& body : bodies) {
            for (auto& other : bodies) {
                if (&body == &other) continue;
                float dx = other.posX - body.posX;
                float dy = other.posY - body.posY;
                float distSq = dx * dx + dy * dy;
                float dist = sqrt(distSq);
                float force = other.mass / distSq; // acceleration, G = 1
                body.velX += force * dx / dist * dt;
                body.velY += force * dy / dist * dt;
            }
        }
    };

};
class Sim {
  public:
    std::vector<Body> bodies;
    Gravity gravity;

    Sim() {}; 
    void addBody(const Body& body) {
        bodies.push_back(body);
    };
    void clearBodies() {
        bodies.clear();
    } 
    void draw() {
      for (const auto& body : bodies) {
          constexpr float twoPi = 6.28318530718f;
          glColor3f(0.95f, 0.35f, 0.25f);
          glBegin(GL_TRIANGLE_FAN);
          glVertex2f(body.posX, body.posY);
          for (int i = 0; i <= 32; ++i) {
              float a = twoPi * i / 32;
              glVertex2f(body.posX + body.radius * std::cos(a), body.posY + body.radius * std::sin(a));
          }
          glEnd();
      }

    }
    void update(float dt) {
        gravity.applyGravity(bodies, dt);
        for (auto& body : bodies) {
            body.posX += body.velX * dt;
            body.posY += body.velY * dt;
        }
    }
};
#endif // SIM_H