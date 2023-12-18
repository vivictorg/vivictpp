// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/OpenGLVideoTextures.hh"
#include "ui/opengl/OpenGLUtils.hh"
#include "ui/opengl/Shaders.hh"
#include "spdlog/spdlog.h"

static const GLfloat vertexBufferData[] = {
        -1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f
};

static const GLfloat uvBufferData[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
};

void vivictpp::ui::YuvRenderer::initFrameBuffer() {

    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(1, &framebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);

    // The texture we're going to render to
    glGenTextures(1, &renderedTexture);
    glActiveTexture(renderedTextureUnit);
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, renderedTexture);

    spdlog::warn("renderedTextureId: {}", renderedTexture);

    // Give an empty image to OpenGL ( the last "0" means "empty" )
    //glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, windowWidth, windowHeight, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
    if (render10bit) {
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB10_A2, renderedResolution.w,renderedResolution.h,
                     0,GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, 0);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, renderedResolution.w,renderedResolution.h,
                     0,GL_RGB, GL_UNSIGNED_BYTE, 0);
    }
    //glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB10_A2, renderedWidth, renderedHeight,
//                     0,GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, 0);

    // Poor filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Failed to create framebuffer");
    }
}

vivictpp::ui::YuvRenderer::YuvRenderer(const VideoMetadata &videoMetadata, GLenum startTextureUnit) :
        inputResolution(videoMetadata.filteredResolution),
        renderedResolution(videoMetadata.filteredResolution.toDisplayResolution(videoMetadata
                                                                                        .filteredSampleAspectRatio)),
        render10bit(false) {
    renderedTextureUnit = startTextureUnit + 3;
    initFrameBuffer();
    programId = vivictpp::ui::opengl::loadShaders( vivictpp::ui::opengl::VERTEX_SHADER_PASTHROUGH,
                                                   vivictpp::ui::opengl::FRAGMENT_SHADER_YUV);

    //LoadShaders( "TransformVertexShader.vertexshader", "yuvShader.fragmentshader" );
    matrixId = glGetUniformLocation(programId, "MVP");
    glGenTextures(3, yuvTextures);
    for (int i = 0; i < 3; i++) {
        yuvTextureUnits[i] = startTextureUnit + i;
        glActiveTexture(yuvTextureUnits[i]);
        glBindTexture(GL_TEXTURE_RECTANGLE_NV, yuvTextures[i]);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    idY = glGetUniformLocation(programId, "s_texture_y");
    idU = glGetUniformLocation(programId, "s_texture_u");
    idV = glGetUniformLocation(programId, "s_texture_v");
    idYScaleFactor = glGetUniformLocation(programId, "scaleFactor");
}

vivictpp::ui::YuvRenderer::~YuvRenderer() {
    glDeleteFramebuffers(1, &framebufferName);
    glDeleteTextures(1, &renderedTexture);
    glDeleteTextures(3, yuvTextures);
    glDeleteProgram(programId);
}

void vivictpp::ui::YuvRenderer::render(glm::mat4 &MVP, GLuint vertexbuffer, GLuint uvbuffer,
                                       GLubyte** planes) {
    int width = inputResolution.w;
    int height = inputResolution.h;
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
    glViewport(0,0,renderedResolution.w,renderedResolution.h); // Render on the whole framebuffer, complete from the
    // lower
    // left corner to
    // the upper right

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float scaleFactor = render10bit ? 32. : 1.;
    glUseProgram(programId);

    glUniform1i(idY, 0);
    glUniform1i(idU, 1);
    glUniform1i(idV, 2);
    glUniform1f(idYScaleFactor, scaleFactor);

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &MVP[0][0]);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
            0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
            1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            2,                                // size : U+V => 2
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
    );

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int i = 0; i < 3; i++) {
        glActiveTexture(yuvTextureUnits[i]);
        GLsizei w = i == 0 ? width : width / 2;
        GLsizei h = i == 0 ? height : height / 2;
        //glPixelStorei(GL_UNPACK_ROW_LENGTH,stride);
        glBindTexture(GL_TEXTURE_RECTANGLE, yuvTextures[i]);
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, render10bit ? GL_R16 : GL_R8,
                     w, h, 0, GL_RED, render10bit ? GL_SHORT : GL_UNSIGNED_BYTE, planes[i]);
        // glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
    }

    glUniform1i(idY, 0);
    glUniform1i(idU, 1);
    glUniform1i(idV, 2);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 2*3); // 12*3 indices starting at 0 -> 12 triangles

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

vivictpp::ui::OpenGLVideoTextures::OpenGLVideoTextures() {
    glm::mat4 Projection = glm::perspective(glm::radians(90.0f), 1.0f / 1.0f, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 View       = glm::lookAt(
            glm::vec3(0,0,1),
            glm::vec3(0,0,0),
            glm::vec3(0,1,0)
    );
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model      = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around


}

vivictpp::ui::OpenGLVideoTextures::~OpenGLVideoTextures() {
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
}

bool vivictpp::ui::OpenGLVideoTextures::update(const vivictpp::ui::DisplayState &displayState) {
    if (displayState.videoMetadataVersion != videoMetadataVersion) {
        if (vertexbuffer == 0) {
            //GLuint vertexbuffer;
            glGenBuffers(1, &vertexbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

            //GLuint uvbuffer;
            glGenBuffers(1, &uvbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(uvBufferData), uvBufferData, GL_STATIC_DRAW);
        }
        initTextures(displayState);
        videoMetadataVersion = displayState.videoMetadataVersion;
    }

    if (leftTextureRenderer && !displayState.leftFrame.empty()) {
        leftTextureRenderer->render(MVP, vertexbuffer, uvbuffer, displayState.leftFrame.avFrame()->data);
    }
    if (rightTextureRenderer && !displayState.rightFrame.empty()) {
        rightTextureRenderer->render(MVP, vertexbuffer, uvbuffer, displayState.rightFrame.avFrame()->data);
    }

    return true;
}

bool vivictpp::ui::OpenGLVideoTextures::initTextures(const vivictpp::ui::DisplayState &displayState) {
    if (videoMetadataVersion == displayState.videoMetadataVersion)
        return false;
    videoTextures.nativeResolution = getNativeResolution(displayState);
    leftTextureRenderer = std::make_unique<YuvRenderer>(displayState.leftVideoMetadata, GL_TEXTURE0);
    videoTextures.leftTexture = static_cast<ImTextureID>((void*) leftTextureRenderer->renderedTexture);
    if (!displayState.rightVideoMetadata.empty()) {
        rightTextureRenderer = std::make_unique<YuvRenderer>(displayState.rightVideoMetadata, GL_TEXTURE4);
        videoTextures.rightTexture = static_cast<ImTextureID>((void*) rightTextureRenderer->renderedTexture);
    }
    return true;
}
