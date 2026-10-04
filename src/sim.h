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
                if (distSq < 1e-12f) continue;
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
    bool showTrails = true;
    float restitution = 0.9f; // 1 = perfectly elastic, 0 = perfectly inelastic
    float radiusScale = 0.464f; // radius = scale * cbrt(mass), i.e. constant density
    size_t maxTrailPoints = 1500;
    int trailInterval = 4; // record a point every N updates
    int stepCount = 0;

    Sim() {}; 
    void syncRadii() {
        for (auto& body : bodies)
            body.radius = radiusScale * std::cbrt(body.mass);
    }
    void addBody(const Body& body) {
        bodies.push_back(body);
        syncRadii();
    };
    void clearBodies() {
        bodies.clear();
    } 
    void drawTrails() {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      for (const auto& body : bodies) {
          size_t n = body.trail.size();
          if (n < 2) continue;
          glBegin(GL_LINE_STRIP);
          for (size_t i = 0; i < n; ++i) {
              float alpha = static_cast<float>(i) / (n - 1);
              glColor4f(0.95f, 0.35f, 0.25f, alpha);
              glVertex2f(body.trail[i].first, body.trail[i].second);
          }
          glEnd();
      }
      glDisable(GL_BLEND);
    }
    void draw() {
      syncRadii();
      if (showTrails) drawTrails();
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
    void resolveCollisions() {
        for (size_t i = 0; i < bodies.size(); ++i) {
            for (size_t j = i + 1; j < bodies.size(); ++j) {
                Body& a = bodies[i];
                Body& b = bodies[j];
                float dx = b.posX - a.posX;
                float dy = b.posY - a.posY;
                float minDist = a.radius + b.radius;
                float distSq = dx * dx + dy * dy;
                if (distSq >= minDist * minDist) continue;

                float dist = std::sqrt(distSq);
                float nx = 1.0f, ny = 0.0f;
                if (dist > 1e-6f) { nx = dx / dist; ny = dy / dist; }

                float invA = a.mass > 0 ? 1.0f / a.mass : 0.0f;
                float invB = b.mass > 0 ? 1.0f / b.mass : 0.0f;
                float invSum = invA + invB;
                if (invSum == 0.0f) continue;

                // Separate the overlap in proportion to inverse mass.
                float overlap = minDist - dist;
                a.posX -= nx * overlap * invA / invSum;
                a.posY -= ny * overlap * invA / invSum;
                b.posX += nx * overlap * invB / invSum;
                b.posY += ny * overlap * invB / invSum;

                // Impulse along the normal, only if the bodies are approaching.
                float relVn = (b.velX - a.velX) * nx + (b.velY - a.velY) * ny;
                if (relVn >= 0.0f) continue;
                float impulse = -(1.0f + restitution) * relVn / invSum;
                a.velX -= impulse * invA * nx;
                a.velY -= impulse * invA * ny;
                b.velX += impulse * invB * nx;
                b.velY += impulse * invB * ny;
            }
        }
    }
    void update(float dt) {
        syncRadii();
        gravity.applyGravity(bodies, dt);
        for (auto& body : bodies) {
            body.posX += body.velX * dt;
            body.posY += body.velY * dt;
        }
        resolveCollisions();
        if (++stepCount % trailInterval == 0) {
            for (auto& body : bodies) {
                body.trail.emplace_back(body.posX, body.posY);
                while (body.trail.size() > maxTrailPoints) body.trail.pop_front();
            }
        }
    }
};
#endif // SIM_H