#include <fstream>
#include <sstream>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>
#else
#include <glad/glad.h>
#endif
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "GlDebug.h"
#include "ExecApi.h"

CShader::CShader(const std::string &VertexPath, const std::string &FragmentPath)
{
   // Read shader files
   std::ifstream vertex_file(VertexPath);
   std::ifstream fragment_file(FragmentPath);

   ExecApiLogMessage("Load vertex shader: %s", VertexPath.c_str());
   ExecApiLogMessage("Load fragment shader: %s", VertexPath.c_str());

   if (!vertex_file.is_open() || !fragment_file.is_open())
   {
      ExecApiLogWarning("Unable to open shader files: %s %s",
         VertexPath.c_str(),
         FragmentPath.c_str());
      exit(EXIT_FAILURE);
   }

   std::stringstream vertex_stream, fragment_stream;
   vertex_stream << vertex_file.rdbuf();
   fragment_stream << fragment_file.rdbuf();

   std::string vertex_code = vertex_stream.str();
   std::string fragment_code = fragment_stream.str();

   const char *vertex_source = vertex_code.c_str();
   const char *fragment_source = fragment_code.c_str();

   // Compile shaders
   unsigned int vertex_shader;
   GLCALL(vertex_shader = glCreateShader(GL_VERTEX_SHADER));
   GLCALL(glShaderSource(vertex_shader, 1, &vertex_source, nullptr));
   GLCALL(glCompileShader(vertex_shader));
   CheckCompileErrors(vertex_shader, "VERTEX");

   unsigned int fragment_shader;
   GLCALL(fragment_shader = glCreateShader(GL_FRAGMENT_SHADER));
   GLCALL(glShaderSource(fragment_shader, 1, &fragment_source, nullptr));
   GLCALL(glCompileShader(fragment_shader));
   CheckCompileErrors(fragment_shader, "FRAGMENT");

   // Link shaders into a program
   GLCALL(mProgramID = glCreateProgram());
   GLCALL(glAttachShader(mProgramID, vertex_shader));
   GLCALL(glAttachShader(mProgramID, fragment_shader));
   GLCALL(glLinkProgram(mProgramID));
   CheckCompileErrors(mProgramID, "PROGRAM");

   // Clean up shaders as they are now linked into the program
   GLCALL(glDeleteShader(vertex_shader));
   GLCALL(glDeleteShader(fragment_shader));
}

CShader::~CShader()
{
   //GLCALL(glDeleteProgram(mProgramID));
}

void CShader::Use() const
{
   GLCALL(glUseProgram(mProgramID));
}

void CShader::SetUniform(const std::string &Name, const glm::mat4 &Matrix) const
{
   int location;
   GLCALL(location = glGetUniformLocation(mProgramID, Name.c_str()));
   if (location == -1)
   {
      ExecApiLogWarning("Uniform '%s' not found in shader", Name.c_str());
      return;
   }
   GLCALL(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(Matrix)));
}

void CShader::SetUniform(const std::string &Name, const glm::vec2 &Vector) const
{
   int location;
   GLCALL(location = glGetUniformLocation(mProgramID, Name.c_str()));
   if (location == -1)
   {
      ExecApiLogWarning("Uniform '%s' not found in shader", Name.c_str());
      return;
   }
   GLCALL(glUniform2fv(location, 1, glm::value_ptr(Vector)));
}

void CShader::SetUniform(const std::string &Name, const glm::vec3 &Vector) const
{
   int location;
   GLCALL(location = glGetUniformLocation(mProgramID, Name.c_str()));
   if (location == -1)
   {
      ExecApiLogWarning("Uniform '%s' not found in shader", Name.c_str());
      return;
   }
   GLCALL(glUniform3fv(location, 1, glm::value_ptr(Vector)));
}

void CShader::SetUniform(const std::string &Name, const glm::vec4 &Vector) const
{
   int location;
   GLCALL(location = glGetUniformLocation(mProgramID, Name.c_str()));
   if (location == -1)
   {
      ExecApiLogWarning("Uniform '%s' not found in shader", Name.c_str());
      return;
   }
   GLCALL(glUniform4fv(location, 1, glm::value_ptr(Vector)));
}

void CShader::SetUniform(const std::string &Name, float Value) const
{
   int location;
   GLCALL(location = glGetUniformLocation(mProgramID, Name.c_str()));
   if (location == -1)
   {
      ExecApiLogWarning("Uniform '%s' not found in shader", Name.c_str());
      return;
   }
   GLCALL(glUniform1f(location, Value));
}

void CShader::SetUniform(const std::string &Name, int Value) const
{
   int location;
   GLCALL(location = glGetUniformLocation(mProgramID, Name.c_str()));
   if (location == -1)
   {
      ExecApiLogWarning("Uniform '%s' not found in shader", Name.c_str());
      return;
   }
   GLCALL(glUniform1i(location, Value));
}

void CShader::CheckCompileErrors(unsigned int Shader, const std::string &Type)
{
   int success;
   char info_log[1024];

   if (Type != "PROGRAM")
   {
      GLCALL(glGetShaderiv(Shader, GL_COMPILE_STATUS, &success));
      if (!success)
      {
         GLCALL(glGetShaderInfoLog(Shader, 1024, nullptr, info_log));
         ExecApiLogWarning("Shader Compilation Error (%s):", Type.c_str());
         ExecApiLogWarning("%s", info_log);
      }
   }
   else
   {
      GLCALL(glGetProgramiv(Shader, GL_LINK_STATUS, &success));
      if (!success)
      {
         GLCALL(glGetProgramInfoLog(Shader, 1024, nullptr, info_log));
         ExecApiLogWarning("Shader Linking Error");
         ExecApiLogWarning("%s", info_log);
      }
   }
}

