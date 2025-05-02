#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include "GlObject.h"

class CGlLineStrip : public CGlObject
{
public:
   const uint32_t MAX_POINTS = 2000;

   // Constructor
   CGlLineStrip(std::shared_ptr<CShader>& Shader, float X, float Y, float Width, float Height);

   void AllowMultipleDrawCalls(bool Enable) { mAllowMultipleDrawCalls = Enable; }

   // Render function
   void Render(const glm::mat4& Projection) override;

   glm::vec4 GetColor() const override;
   void SetColor(const glm::vec4& Color) override;
   void SetLineWidth(float LineWidth) { mLineWidth = LineWidth; }

   void SetVertices(std::vector<glm::vec3>* Vertices);

private:
   void InitBuffers();     // Initialize VAO, VBO

   static unsigned int mVAO;            // Vertex Array Object
   static unsigned int mVBO;            // Vertex Buffer Object
   glm::vec4 mColor;
   float mLineWidth;
   std::vector<glm::vec3>* mVertices;
   bool mAllowMultipleDrawCalls;

   static bool mLineStripInitialized;

};

