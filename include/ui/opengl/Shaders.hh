// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_SHADERS_HH
#define VIVICTPP_SHADERS_HH

#include <string>

namespace vivictpp::ui::opengl {
const std::string FRAGMENT_SHADER_PASTHROUGH =
  R"(
#version 330 core
in vec2 UV;
out vec3 color;
uniform sampler2D renderedTexture;

void main(){
	color = texture( renderedTexture, UV ).xyz ;
}
)";

const std::string VERTEX_SHADER_PASTHROUGH =
        R"(
#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;
out vec2 UV;

void main(){
	gl_Position =  vec4(vertexPosition_modelspace,1);
	UV = (vertexPosition_modelspace.xy+vec2(1,1))/2.0;
}
)";

const std::string FRAGMENT_SHADER_NV12 =
        R"(
#version 330 core

// from here: https://bugfreeblog.duckdns.org/2022/01/yuv420p-opengl-shader-conversion.html

uniform sampler2DRect s_texture_y;
uniform sampler2DRect s_texture_uv;
out vec3 outColor;
in vec2 UV;

void main()
{
  vec2 pos = vec2(UV.x * 1920, UV.y * 800);
  vec2 pU = vec2(2*floor(pos.x / 2), floor(pos.y / 2));
  vec2 pV = vec2(pU.x + 1, pU.y);
  // Why -0.0625 ?
  // Is it because of tv range?
  // From https://stackoverflow.com/questions/20317882/how-can-i-correctly-unpack-a-v210-video-frame-using-glsl
  float Y = texture2DRect(s_texture_y, pos).r - 0.0625;
  float U = texture2DRect(s_texture_uv, pU).r - 0.5;
  float V = texture2DRect(s_texture_uv, pV).r - 0.5;
  vec3 color = vec3(Y, U, V);

  mat3 colorMatrix = mat3(
    1,   0,       1.402,
    1,  -0.344,  -0.714,
    1,   1.772,   0);

  outColor = 1.0 * color*colorMatrix;
})";

const std::string FRAGMENT_SHADER_YUV420 =
        R"(
#version 330 core

// from here: https://bugfreeblog.duckdns.org/2022/01/yuv420p-opengl-shader-conversion.html

uniform sampler2DRect s_texture_y;
uniform sampler2DRect s_texture_u;
uniform sampler2DRect s_texture_v;
uniform float scaleFactor;
out vec3 outColor;
in vec2 UV;

void main()
{
  vec2 pos = vec2(UV.x * 1920, UV.y * 800);
  vec2 p2 = vec2(pos.x / 2, pos.y / 2);
  // Why -0.0625 ?
  // Is it because of tv range?
  // From https://stackoverflow.com/questions/20317882/how-can-i-correctly-unpack-a-v210-video-frame-using-glsl
  float Y = scaleFactor * texture2DRect(s_texture_y, pos).r - 0.0625;
  float U = scaleFactor * texture2DRect(s_texture_u, p2).r - 0.5;
  float V = scaleFactor * texture2DRect(s_texture_v, p2).r - 0.5;
  vec3 color = vec3(Y, U, V);

  mat3 colorMatrix = mat3(
    1,   0,       1.402,
    1,  -0.344,  -0.714,
    1,   1.772,   0);

  outColor = 1.0 * color*colorMatrix;
})";

}
#endif //VIVICTPP_SHADERS_HH
