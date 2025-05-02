
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
#include "Shader.h"
#include "Texture.h"
#include "OpenStreetMap.h"

#define WIDTH  640
#define HEIGHT 480

bool initialize_buffers = true;
GLuint vao;
GLuint vbo;
GLuint ebo;
GLuint program;
glm::vec4 color_ul = glm::vec4(1.0f);
glm::vec4 color_ur = glm::vec4(1.0f);
glm::vec4 color_lr = glm::vec4(1.0f);
glm::vec4 color_ll = glm::vec4(1.0f);
std::shared_ptr<CShader> shader_rect = nullptr;
std::shared_ptr<CShader> shader_line = nullptr;
std::shared_ptr<CTexture> texture = nullptr;
int window_width = WIDTH;
int window_height = HEIGHT;
COpenStreetMap map;

void resize(GLFWwindow* window, int width, int height)
{
   GLCALL(glViewport(0, 0, width, height));
   window_width = width;
   window_height = height;
}

float rect[4] = { 50.0f, 50.0f, 250.0f, 250.0f };

void render()
{
   //float vertices[4][9] =
   //{
   //     // aPos                 // aColor                                       // aTexCoord
   //   { rect[0], rect[1], 0.0f, color_ul.r, color_ul.g, color_ul.b, color_ul.a, 0.0f, 1.0f },
   //   { rect[2], rect[1], 0.0f, color_ur.r, color_ur.g, color_ur.b, color_ur.a, 1.0f, 1.0f },
   //   { rect[2], rect[3], 0.0f, color_lr.r, color_lr.g, color_lr.b, color_lr.a, 1.0f, 0.0f },
   //   { rect[0], rect[3], 0.0f, color_ll.r, color_ll.g, color_ll.b, color_ll.a, 0.0f, 0.0f },
   //};

   //if (initialize_buffers)
   //{
   //   GLuint indices[6] =
   //   {
   //      0, 1, 2,
   //      0, 2, 3,
   //   };

   //   printf("Initialize buffers, program %d\n", program);
   //   GLCALL(glGenVertexArrays(1, &vao));
   //   GLCALL(glGenBuffers(1, &vbo));
   //   GLCALL(glGenBuffers(1, &ebo));

   //   GLCALL(glBindVertexArray(vao));
   //   GLCALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
   //   GLCALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW));
   //   GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
   //   GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
   //   GLCALL(glEnableVertexAttribArray(0));
   //   GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, false, 9 * sizeof(float), (void*)0));
   //   GLCALL(glEnableVertexAttribArray(1));
   //   GLCALL(glVertexAttribPointer(1, 4, GL_FLOAT, false, 9 * sizeof(float), (void*)(3 * sizeof(float))));
   //   GLCALL(glEnableVertexAttribArray(2));
   //   GLCALL(glVertexAttribPointer(2, 2, GL_FLOAT, false, 9 * sizeof(float), (void*)(7 * sizeof(float))));

   //   initialize_buffers = false;
   //}

   //GLCALL(glUseProgram(program));
   //int mvp_loc;
   //GLCALL(mvp_loc = glGetUniformLocation(program, "mvp"));
   //glm::mat4 mvp = glm::ortho(0.0f, (float)window_width, (float)window_height, 0.0f, -1.0f, 1.0f);
   //GLCALL(glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, &mvp[0][0]));

   map.Update();
   map.Draw();

   //if (texture && texture->GetTexture())
   //{
   //   GLCALL(glBindTexture(GL_TEXTURE_2D, texture->GetTexture()));
   //}

   //GLCALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));

   //GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
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
   GLCALL(glClearColor(0.5, 0.5, 0.5, 1.0));

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

      //ImGui::Begin("Debug");
      //ImGui::SliderFloat("Rect X1", &rect[0], 10.0f, 300.0f);
      //ImGui::SliderFloat("Rect X2", &rect[2], 10.0f, 500.0f);
      //ImGui::SliderFloat("Rect Y1", &rect[1], 10.0f, 300.0f);
      //ImGui::SliderFloat("Rect Y2", &rect[3], 10.0f, 500.0f);
      //ImGui::ColorEdit4("Upper Left", &color_ul.r);
      //ImGui::ColorEdit4("Upper Right", &color_ur.r);
      //ImGui::ColorEdit4("Lower Right", &color_lr.r);
      //ImGui::ColorEdit4("Lower Left", &color_ll.r);
      //ImGui::End();

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
