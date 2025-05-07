
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

#define WIDTH              640
#define HEIGHT             480
#define FRAME_RATE         60
#define TIME_CONSTANT      0.2
#define DEGREES_TO_RADIANS M_PI / 180.0

std::shared_ptr<CShader> shader_rect = nullptr;
std::shared_ptr<CShader> shader_line = nullptr;
std::shared_ptr<CTexture> texture = nullptr;
int window_width = WIDTH;
int window_height = HEIGHT;
int map_width = WIDTH;
int map_height = HEIGHT;
COpenStreetMap map;
float map_rotation = 0.0f;
float map_scale_factor = 72000.0f;
int map_offset_x = 0;
int map_offset_y = 0;
double latitude = 38.93916666;
double longitude = -77.46;
double scale = 1.0f;
double mouse_wheel_delta = 0.0;
double dt = 1.0 / FRAME_RATE;
double scale_factor_c1 = exp(-dt / TIME_CONSTANT);
double scale_factor_c2 = 1.0 - scale_factor_c1;
bool draw_boundaries = false;
bool draw_border = false;
bool clip_map = false;
bool enable_easing = false;
bool press_up = false;
bool press_down = false;
bool press_left = false;
bool press_right = false;

void resize(GLFWwindow* window, int width, int height)
{
   GLCALL(glViewport(0, 0, width, height));
   window_width = width;
   window_height = height;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
   mouse_wheel_delta += yoffset;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   switch (key)
   {
      case GLFW_KEY_UP:
         press_up = (action != GLFW_RELEASE);
         break;
      case GLFW_KEY_DOWN:
         press_down = (action != GLFW_RELEASE);
         break;
      case GLFW_KEY_LEFT:
         press_left = (action != GLFW_RELEASE);
         break;
      case GLFW_KEY_RIGHT:
         press_right = (action != GLFW_RELEASE);
         break;
      case GLFW_KEY_Z:
         if (action == GLFW_PRESS && (mods & GLFW_MOD_SHIFT))
            mouse_wheel_delta -= 1.0;
         else if (action == GLFW_PRESS && !(mods & GLFW_MOD_SHIFT))
            mouse_wheel_delta += 1.0;
         break;
      default:
         break;
   }
}

void render()
{
   CGlLineStrip           lines = CGlLineStrip(shader_line, 0.0f, 0.0f, 0.0f, 0.0f);
   CGlRect                rect1 = CGlRect(shader_rect, -25.0f, 25.0f, 50.0f, 50.0f);
   CGlRect                rect2 = CGlRect(shader_rect, 25.0f, -25.0f, 50.0f, 50.0f);
   std::vector<glm::vec3> points;
   glm::mat4              mvp;

   mvp = glm::ortho(-(float)window_width * 0.5f,
                     (float)window_width * 0.5f,
                    -(float)window_height * 0.5f,
                     (float)window_height * 0.5f, -1.0f, 1.0f);

   points.push_back(glm::vec3(-10.0f, -10.0f, 0.0f));
   points.push_back(glm::vec3( 10.0f, -10.0f, 0.0f));
   points.push_back(glm::vec3( 10.0f,  10.0f, 0.0f));
   points.push_back(glm::vec3(-10.0f, -10.0f, 0.0f));
   points.push_back(glm::vec3(-10.0f,  10.0f, 0.0f));
   points.push_back(glm::vec3( 10.0f,  10.0f, 0.0f));
   points.push_back(glm::vec3( 10.0f, -10.0f, 0.0f));
   points.push_back(glm::vec3(-10.0f,  10.0f, 0.0f));

   //rect1.SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
   //rect1.Render(mvp);

   map.SetProjection(mvp);
   map.SetMapCenter(latitude, longitude);
   map.SetMapRotation(map_rotation);
   map.SetMapScaleFactor(map_scale_factor);
   map.EnableEasing(enable_easing);
   map.EnableSubframeBoundaries(draw_boundaries);
   map.EnableBorder(draw_border);
   map.EnableClip(clip_map);
   map.SetMapSize(map_width, map_height);
   map.SetWindowSize(window_width, window_height);
   map.SetMapOffset(map_offset_x, map_offset_y);
   map.Update();
   map.Draw();

   //rect2.SetColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
   //rect2.Render(mvp);

   lines.SetColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
   lines.SetLineWidth(2.0f);
   lines.SetVertices(&points);
   lines.Render(mvp);

   double delta_pos = 0.0005 * (map_scale_factor / 35000.0) * 0.5;
   double longitude_factor = 1.0 / cos(latitude * DEGREES_TO_RADIANS);

   if (press_up)
   {
      latitude += delta_pos * cos(map_rotation * DEGREES_TO_RADIANS);
      longitude += delta_pos * sin(map_rotation * DEGREES_TO_RADIANS) * longitude_factor;
   }
   if (press_down)
   {
      latitude -= delta_pos * cos(map_rotation * DEGREES_TO_RADIANS);
      longitude -= delta_pos * sin(map_rotation * DEGREES_TO_RADIANS) * longitude_factor;
   }
   if (press_left)
   {
      latitude -= delta_pos * cos((map_rotation + 90.0) * DEGREES_TO_RADIANS);
      longitude -= delta_pos * sin((map_rotation + 90.0) * DEGREES_TO_RADIANS) * longitude_factor;
   }
   if (press_right)
   {
      latitude += delta_pos * cos((map_rotation + 90.0) * DEGREES_TO_RADIANS);
      longitude += delta_pos * sin((map_rotation + 90.0) * DEGREES_TO_RADIANS) * longitude_factor;
   }

   // Map scaling
   scale -= (mouse_wheel_delta * 0.01);
   mouse_wheel_delta = 0.0;

   // command scale back to 1.0
   scale = scale_factor_c1 * scale + scale_factor_c2 * 1.0;
   if (fabs(scale - 1.0) < 0.000001)
   {
      scale = 1.0;
   }

   map_scale_factor *= scale;
   if (map_scale_factor < 35000.0f)
      map_scale_factor = 35000.0f;
   else if (map_scale_factor > 10000000.0f)
      map_scale_factor = 10000000.0f;
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
   glfwSetScrollCallback(window, scroll_callback);
   glfwSetKeyCallback(window, key_callback);
   //glfwSwapInterval(1);

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

   // set frame rate
   using framerate = std::chrono::duration<double, std::ratio<1, FRAME_RATE>>;
   auto frame_time = std::chrono::high_resolution_clock::now() + framerate{1};

   map.Open(true, "192.168.1.151:8080", true, "data/map");
   map.SetCoverageRadiusScaleFactor(1.0f);
   map.SetMapRotation(0.0f);
   map.SetBorderColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
   map.SetShaders(shader_rect, shader_line);

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
      ImGui::Checkbox("Draw Border", &draw_border);
      ImGui::SameLine();
      ImGui::Checkbox("Clip", &clip_map);
      ImGui::SameLine();
      ImGui::Checkbox("Draw Boundaries", &draw_boundaries);
      ImGui::Checkbox("Enable Easing", &enable_easing);
      ImGui::Text("Textures loaded: %ld, FPS: %.1f", CTexture::TextureMap.size(), ImGui::GetIO().Framerate);
      ImGui::SliderFloat("Map Rotation", &map_rotation, -180.0f, 180.0f);
      ImGui::SliderFloat("Map Scale Factor", &map_scale_factor, 35000.0f, 10000000.0f);
      ImGui::SliderInt("Map Offset X", &map_offset_x, -500, 500);
      ImGui::SliderInt("Map Offset Y", &map_offset_y, -500, 500);
      ImGui::SliderInt("Map Width", &map_width, WIDTH * 0.5, WIDTH * 2);
      ImGui::SliderInt("Map Height", &map_height, HEIGHT * 0.5, HEIGHT * 2);
      ImGui::Text("Lat/Lon: (%f, %f)", latitude, longitude);
      ImGui::Text("Center tile: %d_%d_%d.png", map.GetZoomLevel(), map.GetCenterTileX(), map.GetCenterTileY());
      ImGui::Text("Zoom Level: %d (%f)", map.GetZoomLevel(), map.GetMapZoom());
      ImGui::Text("Scale: %f", scale);
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
