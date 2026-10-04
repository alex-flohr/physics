#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "sim.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

struct View {
  float centerX = 0.0f, centerY = 5.0f;
  float zoom = 1.0f;
  static constexpr float baseHeight = 10.0f;  // world units visible at zoom 1
  float height() const { return baseHeight / zoom; }
};

// Cursor position in window pixels -> world coordinates.
static void cursorToWorld(GLFWwindow* w, const View& v, double& wx, double& wy) {
  double cx, cy;
  int ww, wh;
  glfwGetCursorPos(w, &cx, &cy);
  glfwGetWindowSize(w, &ww, &wh);
  if (ww <= 0 || wh <= 0) { wx = v.centerX; wy = v.centerY; return; }
  float viewH = v.height();
  float viewW = viewH * static_cast<float>(ww) / wh;
  wx = v.centerX + (cx / ww - 0.5) * viewW;
  wy = v.centerY + (0.5 - cy / wh) * viewH;
}

static void scrollCallback(GLFWwindow* w, double, double dy) {
  if (ImGui::GetIO().WantCaptureMouse) return;
  auto* v = static_cast<View*>(glfwGetWindowUserPointer(w));
  double bx, by;
  cursorToWorld(w, *v, bx, by);
  v->zoom = std::clamp(v->zoom * std::pow(1.1f, static_cast<float>(dy)), 0.1f, 50.0f);
  // Shift the center so the world point under the cursor stays put.
  double ax, ay;
  cursorToWorld(w, *v, ax, ay);
  v->centerX += static_cast<float>(bx - ax);
  v->centerY += static_cast<float>(by - ay);
}

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

  View view;
  glfwSetWindowUserPointer(window, &view);
  glfwSetScrollCallback(window, scrollCallback);  // before ImGui so it gets chained

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
  sim.addBody(Body{-3.0f, 4.0f, 0.0f, 1.826f, 0.01f, 0.3f});
  const std::vector<Body> initialBodies = sim.bodies;

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
    ImGui::SliderFloat("B2 mass", &sim.bodies[1].mass, 0.0f, 20.0f, "%.2f kg");
    ImGui::SliderFloat("Radius scale", &sim.radiusScale, 0.1f, 1.5f, "%.2f");
    if (ImGui::Button("Reset view")) view = View{};
    if (ImGui::Button("Reset")) {
      sim.bodies = initialBodies;
      sim.syncRadii();
      accumulator = 0.0;
    }
    ImGui::End();

    // Left-drag on the scene pans the view.
    static bool dragging = false;
    static double lastX = 0.0, lastY = 0.0;
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    bool down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (down && !dragging && !ImGui::GetIO().WantCaptureMouse) dragging = true;
    if (!down) dragging = false;
    if (dragging) {
      int ww, wh;
      glfwGetWindowSize(window, &ww, &wh);
      if (ww > 0 && wh > 0) {
        float unitsPerPx = view.height() / wh;
        view.centerX -= static_cast<float>(mx - lastX) * unitsPerPx;
        view.centerY += static_cast<float>(my - lastY) * unitsPerPx;
      }
    }
    lastX = mx;
    lastY = my;

    ImGui::Render();
    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Fit the world into the window, preserving aspect ratio.
    float aspect = fbh > 0 ? static_cast<float>(fbw) / fbh : 1.0f;
    float viewH = view.height();
    float viewW = viewH * aspect;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(view.centerX - viewW / 2, view.centerX + viewW / 2,
            view.centerY - viewH / 2, view.centerY + viewH / 2, -1, 1);
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
