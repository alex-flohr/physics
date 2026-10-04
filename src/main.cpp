#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "sim.h"
#include <GLFW/glfw3.h>
#include <cmath>

/*
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
  */

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

  

  double last = glfwGetTime();
  double accumulator = 0.0;
  constexpr double fixedDt = 1.0 / 240.0;
  Sim sim;
  sim.addBody(Body{0.0f, 5.0f, 0.0f, -0.0018f, 10.0f, 1.0f});
  sim.addBody(Body{3.0f, 5.0f, 0.0f, 1.826f, 0.01f, 0.3f});

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    double now = glfwGetTime();
    accumulator += std::fmin(now - last, 0.1);
    last = now;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Physics");

    while (accumulator >= fixedDt) {
      sim.update(static_cast<float>(fixedDt));
      accumulator -= fixedDt;
    }
    ImGui::SliderFloat("B1 mass", &sim.bodies[0].mass, 0.0f, 20.0f, "%.2f kg");
    ImGui::SliderFloat("B1 radius", &sim.bodies[0].radius, 0.1f, 2.5f, "%.2f m");
    ImGui::SliderFloat("B2 mass", &sim.bodies[1].mass, 0.0f, 20.0f, "%.2f kg");
    ImGui::SliderFloat("B2 radius", &sim.bodies[1].radius, 0.1f, 2.5f, "%.2f m");
    ImGui::End();

    ImGui::Render();
    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Fit the world into the window, preserving aspect ratio.
    float aspect = fbh > 0 ? static_cast<float>(fbw) / fbh : 1.0f;
    constexpr float viewH = 10.0f;
    float viewW = viewH * aspect;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-viewW / 2, viewW / 2, 0, viewH, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    sim.draw();

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
