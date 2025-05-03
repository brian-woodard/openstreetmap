#include "GlObject.h"
#include <iomanip>
#include <sstream>
#include "ExecApi.h"

CGlObject::CGlObject(std::shared_ptr<CShader>& Shader, float X, float Y, float Width, float Height)
   : mShader(Shader), mX(X), mY(Y), mWidth(Width), mHeight(Height)
{
}

std::string CGlObject::ColorToHex(const glm::vec4& color)
{
   int r = static_cast<int>(color.r * 255);
   int g = static_cast<int>(color.g * 255);
   int b = static_cast<int>(color.b * 255);
   int a = static_cast<int>(color.a * 255);

   std::ostringstream hexColor;
   hexColor << "#" << std::setfill('0') << std::setw(2) << std::hex << r
            << std::setfill('0') << std::setw(2) << std::hex << g
            << std::setfill('0') << std::setw(2) << std::hex << b
            << std::setfill('0') << std::setw(2) << std::hex << a;

   return hexColor.str();
}

glm::vec4 CGlObject::HexToColor(const std::string& HexColor)
{
   if (HexColor[0] != '#' || HexColor.length() != 9)
   {
      ExecApiLogWarning("Invalid hex color format: '%s'", HexColor.c_str());
      return glm::vec4(1.0f);
   }

   unsigned int r = 0;
   unsigned int g = 0;
   unsigned int b = 0;
   unsigned int a = 0;

   // Parse the color components directly from the string
   int values = std::sscanf(HexColor.c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a);

   if (values != 4)
      return glm::vec4(0.0f);

   // Normalize the values to a range of [0.0, 1.0]
   return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

glm::vec2 CGlObject::GetPosition()
{
   return glm::vec2(mX, mY);
}

glm::vec2 CGlObject::GetSize()
{
   return glm::vec2(mWidth, mHeight);
}

void CGlObject::SetPosition(float NewX, float NewY)
{
   mX = NewX;
   mY = NewY;
}

void CGlObject::SetSize(float NewWidth, float NewHeight)
{
   mWidth = NewWidth;
   mHeight = NewHeight;
}
