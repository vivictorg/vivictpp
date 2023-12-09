// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/opengl/TextureRenderer.hh"
#include "ui/opengl/OpenGLUtils.hh"
#include "ui/opengl/Shaders.hh"
#include <memory>

static const GLfloat vertexBufferData[] = {
    -1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  0.0f,
    -1.0f, -1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f, -1.0f, 0.0f};

static const GLfloat uvBufferData[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
                                       0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f};

glm::mat4 createMvp() {
  glm::mat4 Projection =
      glm::perspective(glm::radians(90.0f), 1.0f / 1.0f, 0.1f, 100.0f);
  // Camera matrix
  glm::mat4 View =
      glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  // Model matrix : an identity matrix (model will be at the origin)
  glm::mat4 Model = glm::mat4(1.0f);
  // Our ModelViewProjection : multiplication of our 3 matrices
  return Projection * View *
         Model; // Remember, matrix multiplication is the other way around
}

GLuint createBuffer(const GLfloat data[], size_t size) {
  GLuint buffer;
  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  return buffer;
}

std::unique_ptr<vivictpp::ui::opengl::TextureRenderer>
vivictpp::ui::opengl::createTextureRenderer(const VideoMetadata &videoMetadata,
                                            GLenum startTextureUnit,
                                            bool render10bit) {
  static glm::mat4 mvp{createMvp()};
  static GLuint vertexbuffer{
      createBuffer(vertexBufferData, sizeof(vertexBufferData))};
  static GLuint uvbuffer{createBuffer(uvBufferData, sizeof(uvBufferData))};

  switch (videoMetadata.filteredPixelFormat) {
  case AV_PIX_FMT_YUV420P:
  case AV_PIX_FMT_YUV420P10LE:
    return std::make_unique<YUV420TextureRenderer>(
        mvp, vertexbuffer, uvbuffer, videoMetadata, startTextureUnit,
        render10bit);
  case AV_PIX_FMT_NV12:
    return std::make_unique<NV12TextureRenderer>(mvp, vertexbuffer, uvbuffer,
                                                 videoMetadata,
                                                 startTextureUnit, render10bit);
  default:
    return nullptr;
  }
}

vivictpp::ui::opengl::TextureRenderer::TextureRenderer(
    glm::mat4 &mvp, GLuint vertexbuffer, GLuint uvbuffer,
    const VideoMetadata &videoMetadata, GLenum startTextureUnit,
    bool render10bit, int textureUnitCount)
    : mvp(mvp), vertexbuffer(vertexbuffer), uvbuffer(uvbuffer),
      renderedTextureUnit(startTextureUnit),
      renderedResolution(videoMetadata.filteredResolution.toDisplayResolution(
          videoMetadata.filteredSampleAspectRatio)),
      inputResolution(videoMetadata.filteredResolution),
      render10bit(render10bit), pixelFormat(videoMetadata.filteredPixelFormat),
      textureUnitCount(textureUnitCount) {
  initFrameBuffer();
}

vivictpp::ui::opengl::TextureRenderer::~TextureRenderer() {
  glDeleteFramebuffers(1, &framebufferName);
  glDeleteTextures(1, &renderedTexture);
}

void vivictpp::ui::opengl::TextureRenderer::initFrameBuffer() {

  // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth
  // buffer.
  glGenFramebuffers(1, &framebufferName);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);

  // The texture we're going to render to
  glGenTextures(1, &renderedTexture);
  glActiveTexture(renderedTextureUnit);
  glBindTexture(GL_TEXTURE_2D, renderedTexture);
  if (render10bit) {
    // Give an empty image to OpenGL ( the last "0" means "empty" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB10_A2, renderedResolution.w,
                 renderedResolution.h, 0, GL_RGBA,
                 GL_UNSIGNED_INT_2_10_10_10_REV, 0);
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderedResolution.w,
                 renderedResolution.h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  }

  // Poor filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture,
                       0);

  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw std::runtime_error("Failed to create framebuffer");
  }
}

vivictpp::ui::opengl::NV12TextureRenderer::NV12TextureRenderer(
    glm::mat4 &mvp, GLuint vertexbuffer, GLuint uvbuffer,
    const VideoMetadata &videoMetadata, GLenum startTextureUnit,
    bool render10bit)
    : TextureRenderer(mvp, vertexbuffer, uvbuffer, videoMetadata,
                      startTextureUnit, render10bit, 3) {
  programId = vivictpp::ui::opengl::loadShaders(
      vivictpp::ui::opengl::VERTEX_SHADER_PASTHROUGH,
      vivictpp::ui::opengl::FRAGMENT_SHADER_NV12);

  // LoadShaders( "TransformVertexShader.vertexshader",
  // "yuvShader.fragmentshader" );
  matrixId = glGetUniformLocation(programId, "MVP");
  glGenTextures(2, yuvTextures);
  for (int i = 0; i < 2; i++) {
    yuvTextureUnits[i] = renderedTextureUnit + i;
    glActiveTexture(yuvTextureUnits[i]);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, yuvTextures[i]);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }
  idY = glGetUniformLocation(programId, "s_texture_y");
  idUV = glGetUniformLocation(programId, "s_texture_uv");
}

vivictpp::ui::opengl::NV12TextureRenderer::~NV12TextureRenderer() {
  glDeleteTextures(2, yuvTextures);
  glDeleteProgram(programId);
}

void vivictpp::ui::opengl::NV12TextureRenderer::render(
    const vivictpp::libav::Frame &frame) {
  GLubyte **planes = frame.avFrame()->data;
  int width = inputResolution.w;
  int height = inputResolution.h;
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
  glViewport(0, 0, renderedResolution.w, renderedResolution.h);

  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // float scaleFactor = render10bit ? 32. : 1.;
  glUseProgram(programId);

  glUniform1i(idY, 0);
  glUniform1i(idUV, 1);

  // Send our transformation to the currently bound shader,
  // in the "MVP" uniform
  glUniformMatrix4fv(matrixId, 1, GL_FALSE, &mvp[0][0]);

  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer(0, // attribute. No particular reason for 0, but must
                           // match the layout in the shader.
                        3, // size
                        GL_FLOAT, // type
                        GL_FALSE, // normalized?
                        0,        // stride
                        (void *)0 // array buffer offset
  );

  // 2nd attribute buffer : UVs
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glVertexAttribPointer(1, // attribute. No particular reason for 1, but must
                           // match the layout in the shader.
                        2, // size : U+V => 2
                        GL_FLOAT, // type
                        GL_FALSE, // normalized?
                        0,        // stride
                        (void *)0 // array buffer offset
  );

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (int i = 0; i < 2; i++) {
    glActiveTexture(yuvTextureUnits[i]);
    GLsizei w = width;
    GLsizei h = i == 0 ? height : height / 2;
    // glPixelStorei(GL_UNPACK_ROW_LENGTH,stride);
    glBindTexture(GL_TEXTURE_RECTANGLE, yuvTextures[i]);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, render10bit ? GL_R16 : GL_R8, w, h, 0,
                 GL_RED, render10bit ? GL_SHORT : GL_UNSIGNED_BYTE, planes[i]);
    // glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
  }

  glUniform1i(idY, 0);
  glUniform1i(idUV, 1);

  // Draw the triangle !
  glDrawArrays(GL_TRIANGLES, 0,
               2 * 3); // 12*3 indices starting at 0 -> 12 triangles

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}

vivictpp::ui::opengl::YUV420TextureRenderer::YUV420TextureRenderer(
    glm::mat4 &mvp, GLuint vertexbuffer, GLuint uvbuffer,
    const VideoMetadata &videoMetadata, GLenum startTextureUnit,
    bool render10bit)
    : TextureRenderer(mvp, vertexbuffer, uvbuffer, videoMetadata,
                      startTextureUnit, render10bit, 4) {
  programId = vivictpp::ui::opengl::loadShaders(
      vivictpp::ui::opengl::VERTEX_SHADER_PASTHROUGH,
      vivictpp::ui::opengl::FRAGMENT_SHADER_YUV420);

  // LoadShaders( "TransformVertexShader.vertexshader",
  // "yuvShader.fragmentshader" );
  matrixId = glGetUniformLocation(programId, "MVP");
  glGenTextures(3, yuvTextures);
  for (int i = 0; i < 3; i++) {
    yuvTextureUnits[i] = renderedTextureUnit + i;
    glActiveTexture(yuvTextureUnits[i]);
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, yuvTextures[i]);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }
  idY = glGetUniformLocation(programId, "s_texture_y");
  idU = glGetUniformLocation(programId, "s_texture_u");
  idV = glGetUniformLocation(programId, "s_texture_v");
  idScaleFactor = glGetUniformLocation(programId, "scaleFactor");
}

vivictpp::ui::opengl::YUV420TextureRenderer::~YUV420TextureRenderer() {
  glDeleteTextures(3, yuvTextures);
  glDeleteProgram(programId);
}

void vivictpp::ui::opengl::YUV420TextureRenderer::render(
    const vivictpp::libav::Frame &frame) {
  GLubyte **planes = frame.avFrame()->data;
  int width = inputResolution.w;
  int height = inputResolution.h;
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
  glViewport(0, 0, renderedResolution.w, renderedResolution.h);

  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // float scaleFactor = render10bit ? 32. : 1.;
  glUseProgram(programId);

  glUniform1i(idY, 0);
  glUniform1i(idU, 1);
  glUniform1i(idV, 2);
  glUniform1f(idScaleFactor, render10bit ? 32.0 : 1.0);

  // Send our transformation to the currently bound shader,
  // in the "MVP" uniform
  glUniformMatrix4fv(matrixId, 1, GL_FALSE, &mvp[0][0]);

  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer(0, // attribute. No particular reason for 0, but must
                           // match the layout in the shader.
                        3, // size
                        GL_FLOAT, // type
                        GL_FALSE, // normalized?
                        0,        // stride
                        (void *)0 // array buffer offset
  );

  // 2nd attribute buffer : UVs
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glVertexAttribPointer(1, // attribute. No particular reason for 1, but must
                           // match the layout in the shader.
                        2, // size : U+V => 2
                        GL_FLOAT, // type
                        GL_FALSE, // normalized?
                        0,        // stride
                        (void *)0 // array buffer offset
  );

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (int i = 0; i < 3; i++) {
    glActiveTexture(yuvTextureUnits[i]);
    GLsizei w = i == 0 ? width : width / 2;
    GLsizei h = i == 0 ? height : height / 2;
    // glPixelStorei(GL_UNPACK_ROW_LENGTH,stride);
    glBindTexture(GL_TEXTURE_RECTANGLE, yuvTextures[i]);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, render10bit ? GL_R16 : GL_R8, w, h, 0,
                 GL_RED, render10bit ? GL_SHORT : GL_UNSIGNED_BYTE, planes[i]);
    // glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
  }

  glUniform1i(idY, 0);
  glUniform1i(idU, 1);
  glUniform1i(idV, 2);

  // Draw the triangle !
  glDrawArrays(GL_TRIANGLES, 0,
               2 * 3); // 12*3 indices starting at 0 -> 12 triangles

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}
