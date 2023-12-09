// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_TEXTURERENDERER_HH
#define VIVICTPP_TEXTURERENDERER_HH

#include "Resolution.hh"
#include "VideoMetadata.hh"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "libav/Frame.hh"
#include <GL/glew.h>
#include <memory>

namespace vivictpp::ui::opengl {
class TextureRenderer {
protected:
  glm::mat4 mvp;
  GLuint vertexbuffer;
  GLuint uvbuffer;
  GLuint framebufferName{0};
  GLuint renderedTexture;
  GLenum renderedTextureUnit;
  Resolution renderedResolution;
  Resolution inputResolution;
  bool render10bit;
  AVPixelFormat pixelFormat;
  int textureUnitCount;

public:
  virtual ~TextureRenderer();
  virtual void render(const vivictpp::libav::Frame &frame) = 0;
  GLuint getRenderedTextureId() const { return renderedTexture; }
  int getTextureUnitCount() { return textureUnitCount; }

protected:
  TextureRenderer(glm::mat4 &mvp, GLuint vertexbuffer, GLuint uvbuffer,
                  const VideoMetadata &videoMetadata, GLenum startTextureUnit,
                  bool render10bit, int textureUnitCount);

private:
  void initFrameBuffer();
};

class NV12TextureRenderer : public TextureRenderer {
private:
  GLuint programId;
  GLuint yuvTextures[2];
  GLuint idY;
  GLuint idUV;
  GLuint matrixId;
  GLenum yuvTextureUnits[2];

public:
  NV12TextureRenderer(glm::mat4 &mvp, GLuint vertexbuffer, GLuint uvbuffer,
                      const VideoMetadata &videoMetadata,
                      GLenum startTextureUnit, bool render10bit);
  ~NV12TextureRenderer() override;
  virtual void render(const vivictpp::libav::Frame &frame) override;
};

class YUV420TextureRenderer : public TextureRenderer {
private:
  GLuint programId;
  GLuint yuvTextures[3];
  GLuint idY;
  GLuint idU;
  GLuint idV;
  GLuint idScaleFactor;
  GLuint matrixId;
  GLenum yuvTextureUnits[3];

public:
  YUV420TextureRenderer(glm::mat4 &mvp, GLuint vertexbuffer, GLuint uvbuffer,
                        const VideoMetadata &videoMetadata,
                        GLenum startTextureUnit, bool render10bit);
  ~YUV420TextureRenderer() override;
  virtual void render(const vivictpp::libav::Frame &frame) override;
};

std::unique_ptr<TextureRenderer>
createTextureRenderer(const VideoMetadata &videoMetadata,
                      GLenum startTextureUnit, bool render10bit);

} // namespace vivictpp::ui::opengl

#endif // VIVICTPP_TEXTURERENDERER_HH
