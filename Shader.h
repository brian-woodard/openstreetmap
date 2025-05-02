#pragma once

#include <string>
#include <glm/glm.hpp>

class CShader
{
public:
   CShader(const std::string &VertexPath, const std::string &FragmentPath);
   ~CShader();

   void Use() const;
   unsigned int GetProgramID() const { return mProgramID; }
   //void LoadShaders(const std::string& VertexPath, const std::string& FragmentPath);

   // SetUniform functions
   void SetUniform(const std::string &Name, const glm::mat4 &Matrix) const;
   void SetUniform(const std::string &Name, const glm::vec2 &Vector) const;
   void SetUniform(const std::string &Name, const glm::vec3 &Vector) const;
   void SetUniform(const std::string &Name, const glm::vec4 &Vector) const;
   void SetUniform(const std::string &Name, float Value) const;
   void SetUniform(const std::string &Name, int Value) const;

private:
   void CheckCompileErrors(unsigned int Shader, const std::string &Type);

   unsigned int mProgramID;
};

