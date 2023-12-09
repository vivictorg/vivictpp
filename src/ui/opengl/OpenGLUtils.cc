// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/opengl/OpenGLUtils.hh"

#include "logging/Logging.hh"
#include <vector>

GLuint
vivictpp::ui::opengl::loadShaders(const std::string &vertexShaderCode,
                                  const std::string &fragmentShaderCode) {

  auto logger =
      vivictpp::logging::getOrCreateLogger("vivictpp::ui::opengl::loadShaders");

  // Create the shaders
  GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

  GLint result = GL_FALSE;
  int infoLogLength;

  char const *vertexSourcePointer = vertexShaderCode.c_str();
  glShaderSource(vertexShaderID, 1, &vertexSourcePointer, NULL);
  glCompileShader(vertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
  glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (infoLogLength > 0) {
    std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
    glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL,
                       &vertexShaderErrorMessage[0]);
    logger->warn("Error loading vertex shader: {}",
                 &vertexShaderErrorMessage[0]);
  }

  char const *FragmentSourcePointer = fragmentShaderCode.c_str();
  glShaderSource(fragmentShaderId, 1, &FragmentSourcePointer, NULL);
  glCompileShader(fragmentShaderId);

  // Check Fragment Shader
  glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &result);
  glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (infoLogLength > 0) {
    std::vector<char> fragmentShaderErrorMessage(infoLogLength + 1);
    glGetShaderInfoLog(fragmentShaderId, infoLogLength, NULL,
                       &fragmentShaderErrorMessage[0]);
    logger->warn("Error loading fragment shader: {}",
                 &fragmentShaderErrorMessage[0]);
  }

  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, vertexShaderID);
  glAttachShader(ProgramID, fragmentShaderId);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);
  if (infoLogLength > 0) {
    std::vector<char> programErrorMessage(infoLogLength + 1);
    glGetProgramInfoLog(ProgramID, infoLogLength, NULL,
                        &programErrorMessage[0]);
    logger->warn("Error linkng program: {}", &programErrorMessage[0]);
  }

  glDetachShader(ProgramID, vertexShaderID);
  glDetachShader(ProgramID, fragmentShaderId);

  glDeleteShader(vertexShaderID);
  glDeleteShader(fragmentShaderId);

  return ProgramID;
}
