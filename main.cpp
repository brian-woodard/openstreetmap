
#include <stdio.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "GlDebug.h"
#include "GlLineStrip.h"
#include "GlRect.h"
#include "Shader.h"
#include "Texture.h"
#include "OpenStreetMap.h"

#define WIDTH  640
#define HEIGHT 480

std::shared_ptr<CShader> shader_rect = nullptr;
std::shared_ptr<CShader> shader_line = nullptr;
std::shared_ptr<CTexture> texture = nullptr;
int window_width = WIDTH;
int window_height = HEIGHT;
COpenStreetMap map;
float map_rotation = 0.0f;

void resize(GLFWwindow* window, int width, int height)
{
   GLCALL(glViewport(0, 0, width, height));
   window_width = width;
   window_height = height;
}

float rect[4] = { 50.0f, 50.0f, 250.0f, 250.0f };

void render()
{
   CGlLineStrip           lines = CGlLineStrip(shader_line, 0.0f, 0.0f, 0.0f, 0.0f);
   CGlRect                rect = CGlRect(shader_rect, 0.0f, 0.0f, 100.0f, 100.0f);
   std::vector<glm::vec3> points;
   glm::mat4              mvp;

   mvp = glm::ortho(-(float)window_width * 0.5f,
                     (float)window_width * 0.5f,
                    -(float)window_height * 0.5f,
                     (float)window_height * 0.5f, -1.0f, 1.0f);

   points.push_back(glm::vec3(-50.0f, -50.0f, 0.0f));
   points.push_back(glm::vec3( 50.0f, -50.0f, 0.0f));
   points.push_back(glm::vec3( 50.0f,  50.0f, 0.0f));
   points.push_back(glm::vec3(-50.0f,  50.0f, 0.0f));
   points.push_back(glm::vec3(-50.0f, -50.0f, 0.0f));

   map.SetProjection(mvp);
   map.SetMapRotation(map_rotation);
   map.Update();
   map.Draw();

   rect.SetColor(glm::vec4(1.0f));
   rect.SetTexture(texture);
   rect.Render(mvp);

   lines.SetColor(glm::vec4(1.0f));
   lines.SetVertices(&points);
   lines.Render(mvp);
}

int main(int argc, char* argv[])
{
   GLFWwindow* window = nullptr;

   // initialize glfw
   if (!glfwInit())
      return 0;

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   // Create window
   window = glfwCreateWindow(WIDTH, HEIGHT, "Open Street Map Demo", NULL, NULL);

   if (!window)
   {
      glfwTerminate();
      return 0;
   }

   // make the window's context current
   glfwMakeContextCurrent(window);

   // use glad to load OpenGL function pointers
   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
   {
      printf("Error: Failed to initialize GLAD.\n");
      glfwTerminate();
      return 0;
   }

   glfwSetWindowSize(window, WIDTH, HEIGHT);

   // Setup Dear ImGui
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui::StyleColorsDark();

   // Setup Platform/Render backends
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init("#version 330");

   // Make the window visible
   glfwShowWindow(window);

   // Initialize opengl
   GLCALL(glClearColor(0.5f, 0.5f, 0.5f, 1.0f));

   // enable blending
   GLCALL(glEnable(GL_BLEND));
   GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

   glfwSetWindowSizeCallback(window, resize);

   // Load shaders
   shader_rect = std::make_shared<CShader>("data/shaders/rect.vert", "data/shaders/rect.frag");
   shader_line = std::make_shared<CShader>("data/shaders/line.vert", "data/shaders/line.frag");

   texture = GetOrCreateTexture("logo_icon.png");

   // set frame rate to 60 Hz
   using framerate = std::chrono::duration<double, std::ratio<1, 60>>;
   auto frame_time = std::chrono::high_resolution_clock::now() + framerate{1};

   map.Open(false, "", true, "data");
   map.SetCoverageRadiusScaleFactor(1.0f);
   map.SetMapCenter(38.93916666, -77.46);
   map.SetMapRotation(0.0f);
   map.SetMapScaleFactor(10000000.0f);
   map.SetMapSize(WIDTH, HEIGHT);
   map.SetShaders(shader_rect, shader_line);
   map.EnableSubframeBoundaries(true);

   while (window)
   {
      // Poll events
      glfwPollEvents();

      if (glfwWindowShouldClose(window))
      {
         CTexture::DeleteTextures();
         glfwDestroyWindow(window);
         glfwTerminate();
         window = nullptr;
         break;
      }

      GLCALL(glClear(GL_COLOR_BUFFER_BIT));

      render();

      // Start the Dear ImGui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      ImGui::Begin("Debug");
      ImGui::SliderFloat("Map Rotation", &map_rotation, -180.0f, 180.0f);
      ImGui::End();

      // Render ImGui
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);

      // wait until next frame
      std::this_thread::sleep_until(frame_time);
      frame_time += framerate{1};
   }

   return 0;
}
