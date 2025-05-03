#include "GlRect.h"
#include "GlDebug.h"
#include <glm/gtc/matrix_transform.hpp>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>
#else
#include <glad/glad.h>
#endif
#include "ExecApi.h"

unsigned int CGlRect::mVAO = 0;
unsigned int CGlRect::mVBO = 0;
unsigned int CGlRect::mEBO = 0;
bool CGlRect::mRectInitialized = false;

// Constructor
CGlRect::CGlRect(std::shared_ptr<CShader>& Shader, float X, float Y, float Width, float Height)
   : CGlObject(Shader, X, Y, Width, Height),
     mModel(1.0f),
     mBorderColor(0.0f), mBorderThickness(0.0f), mCornerRadius(0.0f), mEdgeSoftness(0.0f), mTexture(nullptr)
{
   if (!mRectInitialized)
      InitBuffers();
}

void CGlRect::InitBuffers()
{
   unsigned int indices[] = {
      0, 1, 2,
      2, 3, 0
   };

   GLCALL(glGenVertexArrays(1, &mVAO));
   GLCALL(glGenBuffers(1, &mVBO));
   GLCALL(glGenBuffers(1, &mEBO));

   GLCALL(glBindVertexArray(mVAO));

   GLCALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
   GLCALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 9 * 4, nullptr, GL_DYNAMIC_DRAW));

   GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO));
   GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW));

   GLCALL(glEnableVertexAttribArray(0));
   GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0));
   GLCALL(glEnableVertexAttribArray(1));
   GLCALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float))));
   GLCALL(glEnableVertexAttribArray(2));
   GLCALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float))));

   GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
   GLCALL(glBindVertexArray(0));

   mRectInitialized = true;
}

glm::vec4 CGlRect::GetColor() const
{
   return mColor[0];
}

void CGlRect::SetBorderColor(const glm::vec4& Color)
{
   mBorderColor = Color;
}

void CGlRect::SetBorderThickness(float Thickness)
{
   mBorderThickness = Thickness;
}

void CGlRect::SetColor(const glm::vec4& Color)
{
   mColor[0] = Color;
   mColor[1] = Color;
   mColor[2] = Color;
   mColor[3] = Color;
}

void CGlRect::SetColorLL(const glm::vec4& Color)
{
   mColor[0] = Color;
}

void CGlRect::SetColorLR(const glm::vec4& Color)
{
   mColor[1] = Color;
}

void CGlRect::SetColorUR(const glm::vec4& Color)
{
   mColor[2] = Color;
}

void CGlRect::SetColorUL(const glm::vec4& Color)
{
   mColor[3] = Color;
}

void CGlRect::SetColors(const glm::vec4 Color[])
{
   mColor[0] = Color[0];
   mColor[1] = Color[1];
   mColor[2] = Color[2];
   mColor[3] = Color[3];
}

void CGlRect::SetCornerRadius(float Radius)
{
   mCornerRadius = Radius;
}

void CGlRect::SetEdgeSoftness(float Softness)
{
   mEdgeSoftness = Softness;
}

void CGlRect::SetModelMatrix(const glm::mat4& Model)
{
   mModel = Model;
}

void CGlRect::SetTranslate(const glm::vec3& Translate)
{
   mModel = glm::translate(mModel, Translate);
}

void CGlRect::SetRotation(float Rotation)
{
   mModel = glm::rotate(mModel, Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
}

void CGlRect::SetTexture(std::shared_ptr<CTexture> Texture)
{
   mTexture = Texture;
}

// Render the polygon
void CGlRect::Render(const glm::mat4& Projection)
{
   if (!mShader || !mVAO)
   {
      ExecApiLogWarning("CGlRect: Shader or VAO is not initialized!");
      return;
   }

   glm::vec2 vertices_ll;
   glm::vec2 vertices_ur;

   mShader->Use();
   GLCALL(glBindVertexArray(mVAO));

   // Set the transformation matrix
   glm::mat4 transform = Projection;

   // Rectangle drawing has rotation centered
   vertices_ll = glm::vec2(-mWidth * 0.5f, -mHeight * 0.5f);
   vertices_ur = glm::vec2( mWidth * 0.5f,  mHeight * 0.5f);

   transform = glm::translate(transform, glm::vec3(mX, mY, 0.0f));
   transform *= mModel;

   mShader->SetUniform("transform", transform);

   // Set the texture (if available)
   if (mTexture && mTexture->GetTexture())
   {
      mShader->SetUniform("uSampleTexture", 1);
      glBindTexture(GL_TEXTURE_2D, mTexture->GetTexture());
   }
   else
   {
      mShader->SetUniform("uSampleTexture", 0);
   }

   mShader->SetUniform("uBorderColor", mBorderColor);
   mShader->SetUniform("uBorderThickness", mBorderThickness);
   mShader->SetUniform("uCornerRadius", mCornerRadius);
   mShader->SetUniform("uEdgeSoftness", mEdgeSoftness);

   // Set rect size uniform
   glm::vec2 rect = glm::vec2(mWidth, mHeight);
   mShader->SetUniform("uRectSize", rect);

   // Texture coordinates
   glm::vec2 tex_coords_ll = glm::vec2(0.0f, 0.0f);
   glm::vec2 tex_coords_ur = glm::vec2(1.0f, 1.0f);

   // Update vertex buffer data
   float vertices[4][9] = {
        // vertex                           color                                               texture coords
      { vertices_ll.x, vertices_ll.y, 0.0f, mColor[0].r, mColor[0].g, mColor[0].b, mColor[0].a, tex_coords_ll.x, tex_coords_ll.y },
      { vertices_ur.x, vertices_ll.y, 0.0f, mColor[1].r, mColor[1].g, mColor[1].b, mColor[1].a, tex_coords_ur.x, tex_coords_ll.y },
      { vertices_ur.x, vertices_ur.y, 0.0f, mColor[2].r, mColor[2].g, mColor[2].b, mColor[2].a, tex_coords_ur.x, tex_coords_ur.y },
      { vertices_ll.x, vertices_ur.y, 0.0f, mColor[3].r, mColor[3].g, mColor[3].b, mColor[3].a, tex_coords_ll.x, tex_coords_ur.y },
   };

   GLCALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
   GLCALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));

   GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
}
