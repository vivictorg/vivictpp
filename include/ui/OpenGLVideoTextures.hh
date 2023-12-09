// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_OPENGLVIDEOTEXTURES_HH
#define VIVICTPP_OPENGLVIDEOTEXTURES_HH

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <GL/glew.h>

#include "DisplayState.hh"
#include "VideoTextures.hh"
#include "ui/opengl/TextureRenderer.hh"

namespace vivictpp::ui {

class YuvRenderer {
  friend class OpenGLVideoTextures;

private:
  void initFrameBuffer();

public:
  YuvRenderer(const VideoMetadata &videoMetadata, GLenum startTextureUnit);

  ~YuvRenderer();

  void render(glm::mat4 &MVP, GLuint vertexbuffer, GLuint uvbuffer,
              GLubyte **planes);

private:
  GLuint programId;
  GLuint yuvTextures[3];
  GLuint idY;
  GLuint idU;
  GLuint idV;
  GLuint idYScaleFactor;
  GLuint idUScaleFactor;
  GLuint idVScaleFactor;
  GLuint matrixId;
  GLuint framebufferName{0};
  GLuint renderedTexture;
  GLenum yuvTextureUnits[3];
  GLenum renderedTextureUnit;
  Resolution renderedResolution;
  Resolution inputResolution;
  bool render10bit;
  bool input10bit;
};

class OpenGLVideoTextures {
public:
  OpenGLVideoTextures();
  ~OpenGLVideoTextures();
  bool update(const DisplayState &displayState);
  VideoTextures &getVideoTextures() { return videoTextures; }

private:
  bool initTextures(const DisplayState &displayState);

private:
  std::unique_ptr<vivictpp::ui::opengl::TextureRenderer> leftTextureRenderer;
  std::unique_ptr<vivictpp::ui::opengl::TextureRenderer> rightTextureRenderer;
  vivictpp::ui::VideoTextures videoTextures;
  int videoMetadataVersion{-1};
  glm::mat4 MVP;
  GLuint vertexbuffer{0};
  GLuint uvbuffer{0};
};

} // namespace vivictpp::ui

#endif // VIVICTPP_OPENGLVIDEOTEXTURES_HH
