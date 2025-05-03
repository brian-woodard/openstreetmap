#pragma once

#include <memory>
#include "GlObject.h"
#include "Texture.h"

class CGlRect : public CGlObject
{
public:
   // Constructor
   CGlRect(std::shared_ptr<CShader>& Shader, float X, float Y, float Width, float Height);

   // Render function
   void Render(const glm::mat4& Projection) override;

   glm::vec4 GetColor() const override;
   void SetColor(const glm::vec4& Color) override;
   void SetColorLL(const glm::vec4& Color);
   void SetColorLR(const glm::vec4& Color);
   void SetColorUR(const glm::vec4& Color);
   void SetColorUL(const glm::vec4& Color);
   void SetColors(const glm::vec4 Color[]);

   void SetBorderColor(const glm::vec4& Color);
   void SetBorderThickness(float Thickness);
   void SetCornerRadius(float Radius);
   void SetEdgeSoftness(float Softness);

   void SetModelMatrix(const glm::mat4& Model);

   void SetTranslate(const glm::vec3& Translate);
   void SetRotation(float Rotation);

   void SetTexture(std::shared_ptr<CTexture> Texture);

private:
   void InitBuffers();     // Initialize VAO, VBO, and EBO

   static unsigned int mVAO;            // Vertex Array Object
   static unsigned int mVBO;            // Vertex Buffer Object
   static unsigned int mEBO;            // Element Buffer Object for polygons
   glm::mat4 mModel;
   glm::vec4 mColor[4];
   glm::vec4 mBorderColor;
   float mBorderThickness;
   float mCornerRadius;
   float mEdgeSoftness;
   std::shared_ptr<CTexture> mTexture;

   static bool mRectInitialized;

};

