#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cmath>

struct Ball {
  float x = 0.0f, y = 4.0f;    // meters
  float vx = 2.0f, vy = 0.0f;  // m/s
  float radius = 0.3f;         // meters
};

struct World {
  float gravity = -9.81f;      // m/s^2
  float restitution = 0.85f;   // energy kept per bounce
  float halfWidth = 5.0f;      // world spans [-halfWidth, halfWidth]
  float height = 6.0f;         // world spans [0, height]
};

void step(Ball& b, const World& w, float dt) {
  b.vy += w.gravity * dt;
  b.x += b.vx * dt;
  b.y += b.vy * dt;

  if (b.y - b.radius < 0.0f) {
    b.y = b.radius;
    b.vy = -b.vy * w.restitution;
  }
  if (b.y + b.radius > w.height) {
    b.y = w.height - b.radius;
    b.vy = -b.vy * w.restitution;
  }
  if (b.x - b.radius < -w.halfWidth) {
    b.x = -w.halfWidth + b.radius;
    b.vx = -b.vx * w.restitution;
  }
  if (b.x + b.radius > w.halfWidth) {
    b.x = w.halfWidth - b.radius;
    b.vx = -b.vx * w.restitution;
  }
}

void drawBall(const Ball& b) {
  constexpr int segments = 48;
  constexpr float twoPi = 6.28318530718f;
  glColor3f(0.95f, 0.35f, 0.25f);
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(b.x, b.y);
  for (int i = 0; i <= segments; ++i) {
    float a = twoPi * i / segments;
    glVertex2f(b.x + b.radius * std::cos(a), b.y + b.radius * std::sin(a));
  }
  glEnd();
}

int main() {
  if (!glfwInit())
    return 1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  GLFWwindow* window = glfwCreateWindow(1280, 800, "Physics", nullptr, nullptr);
  if (window == nullptr) {
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  World world;
  Ball ball;
  const Ball initial = ball;
  double last = glfwGetTime();
  double accumulator = 0.0;
  constexpr double fixedDt = 1.0 / 240.0;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    double now = glfwGetTime();
    accumulator += std::fmin(now - last, 0.1);
    last = now;
    while (accumulator >= fixedDt) {
      step(ball, world, static_cast<float>(fixedDt));
      accumulator -= fixedDt;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Physics");
    ImGui::SliderFloat("Gravity", &world.gravity, -20.0f, 0.0f, "%.2f m/s^2");
    ImGui::SliderFloat("Ball radius", &ball.radius, 0.1f, 2.5f, "%.2f m");
    ImGui::SliderFloat("Restitution", &world.restitution, 0.0f, 1.0f);
    if (ImGui::Button("Reset"))
      ball = initial;
    ImGui::End();

    ImGui::Render();
    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Fit the world into the window, preserving aspect ratio.
    float aspect = fbh > 0 ? static_cast<float>(fbw) / fbh : 1.0f;
    float viewH = world.height;
    float viewW = viewH * aspect;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-viewW / 2, viewW / 2, 0, viewH, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawBall(ball);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
