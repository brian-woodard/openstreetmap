#version 330 core
layout(location = 0) in vec3 aPos;  // Position
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 transform;

out vec4 Color;
out vec2 TexCoords;

void main()
{
   gl_Position = transform * vec4(aPos, 1.0);
   Color = aColor;
   TexCoords = aTexCoord;
}
