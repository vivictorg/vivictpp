// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_OPENGLUTILS_HH
#define VIVICTPP_OPENGLUTILS_HH

#include <GL/glew.h>
#include <string>

namespace vivictpp::ui::opengl {
GLuint loadShaders(const std::string &vertexShaderCode,
                   const std::string &fragmentShaderCode);
}

#endif // VIVICTPP_OPENGLUTILS_HH
