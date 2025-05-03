#pragma once

#include "Shader.h"
#include <glm/glm.hpp>
#include <memory>

class CGlObject
{
public:

   explicit CGlObject(std::shared_ptr<CShader>& Shader, float X, float Y, float Width, float Height);
   virtual ~CGlObject() = default;

   // Set position and size
   glm::vec2 GetPosition();
   glm::vec2 GetSize();
   void SetPosition(float X, float Y);
   void SetSize(float Width, float Height);

   //Overides for polygons
   virtual glm::vec4 GetColor() const { return {}; }
   virtual void SetColor(const glm::vec4&) {}

   //Overrides for text
   virtual std::string GetText() const { return {}; }
   virtual void SetText(const std::string&) {}

   // Virtual render method to be implemented by subclasses
   virtual void Render(const glm::mat4& Projection) = 0;

   static std::string ColorToHex(const glm::vec4& color);
   static glm::vec4 HexToColor(const std::string& HexColor);

protected:
   std::shared_ptr<CShader> mShader;  // Shared shader for rendering
   float mX, mY;                      // Position as percentages
   float mWidth, mHeight;             // Size as percentages

};

