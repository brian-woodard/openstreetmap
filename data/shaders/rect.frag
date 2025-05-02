#version 330 core
uniform sampler2D uTexture;
uniform int uSampleTexture = 0;
uniform vec4 uBorderColor;
uniform float uBorderThickness;
uniform float uCornerRadius;
uniform float uEdgeSoftness;
uniform vec2 uRectSize;

in vec4 Color;
in vec2 TexCoords;

float RectSDF(vec2 p, vec2 b, float r)
{
   vec2 d = abs(p) - b + vec2(r);
   return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;
}

void main()
{
   vec4 sample = vec4(1.0f);
   vec2 pos = uRectSize * TexCoords;
   vec4 oColor;

   if (uSampleTexture == 1)
   {
      sample = texture(uTexture, TexCoords) * Color;
   }

   vec2 softness_padding = vec2(max(0.0, uEdgeSoftness*2.0),
                                max(0.0, uEdgeSoftness*2.0));

   float dist = RectSDF(pos-uRectSize/2.0, uRectSize/2.0 - softness_padding, uCornerRadius);
   float sdf_factor = 1.0f - smoothstep(0.0, 2.0*uEdgeSoftness, dist);

   if (uBorderThickness > 0.0)
   {
      dist = RectSDF(pos-uRectSize/2.0, uRectSize/2.0 - uBorderThickness/2.0, uCornerRadius);
      float blend_amount = smoothstep(0.0, 1.0, abs(dist) - uBorderThickness / 2.0);
      vec4 from_color = uBorderColor;
      vec4 to_color = (dist <= 0.0) ? Color : vec4(0.0);
      oColor = mix(from_color, to_color, blend_amount) * sample * sdf_factor;
   }
   else
   {
      oColor = Color * sample * sdf_factor;
   }

   gl_FragColor = oColor;
}
