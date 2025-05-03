#include "GlLineStrip.h"
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

unsigned int CGlLineStrip::mVAO = 0;
unsigned int CGlLineStrip::mVBO = 0;
bool CGlLineStrip::mLineStripInitialized = false;

// Constructor
CGlLineStrip::CGlLineStrip(std::shared_ptr<CShader>& Shader, float X, float Y, float Width, float Height)
   : CGlObject(Shader, X, Y, Width, Height),
     mModel(1.0f),
     mLineWidth(1.0f), mVertices(nullptr), mAllowMultipleDrawCalls(true)
{
   if (!mLineStripInitialized)
      InitBuffers();
}

void CGlLineStrip::InitBuffers()
{
   GLCALL(glGenVertexArrays(1, &mVAO));
   GLCALL(glGenBuffers(1, &mVBO));

   GLCALL(glBindVertexArray(mVAO));

   GLCALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
   GLCALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * MAX_POINTS, nullptr, GL_DYNAMIC_DRAW));

   GLCALL(glEnableVertexAttribArray(0));
   GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));

   GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
   GLCALL(glBindVertexArray(0));

   mLineStripInitialized = true;
}

glm::vec4 CGlLineStrip::GetColor() const
{
   return mColor;
}

void CGlLineStrip::SetColor(const glm::vec4& Color)
{
   mColor = Color;
}

void CGlLineStrip::SetModelMatrix(const glm::mat4& Model)
{
   mModel = Model;
}

void CGlLineStrip::SetRotation(float Rotation)
{
   mModel = glm::rotate(mModel, Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
}

void CGlLineStrip::SetVertices(std::vector<glm::vec3>* Vertices)
{
   mVertices = Vertices;
}

void CGlLineStrip::Render(const glm::mat4& Projection)
{
   if (!mShader || !mVAO)
   {
      ExecApiLogWarning("CGlLineStrip: Shader or VAO is not initialized!");
      return;
   }

   if (!mVertices || mVertices->size() < 1)
      return;

   mShader->Use();
   GLCALL(glBindVertexArray(mVAO));

   // Set the transformation matrix
   glm::mat4 transform = Projection * mModel;

   mShader->SetUniform("transform", transform);
   mShader->SetUniform("uColor", mColor);

   GLCALL(glLineWidth(mLineWidth));

   GLCALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));

   if (mVertices->size() <= MAX_POINTS)
   {
      // Update vertex buffer data
      GLCALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * mVertices->size(), mVertices->data()));
      GLCALL(glDrawArrays(GL_LINE_STRIP, 0, mVertices->size()));
   }
   else
   {
      size_t start = 0;
      size_t end = MAX_POINTS-1;

      while (start < mVertices->size() - 1)
      {
         int num_vertices = end - start + 1;
         int size = sizeof(glm::vec3) * num_vertices;

         // Update vertex buffer data
         GLCALL(glBufferSubData(GL_ARRAY_BUFFER, 0, size, &mVertices->data()[start]));
         GLCALL(glDrawArrays(GL_LINE_STRIP, 0, num_vertices));

         start = end;
         end += MAX_POINTS-1;

         if (end > mVertices->size() - 1)
            end = mVertices->size() - 1;

         if (!mAllowMultipleDrawCalls)
            break;
      }
   }
}
