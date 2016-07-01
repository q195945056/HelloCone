//
//  RenderingEngine2.cpp
//  HelloCone
//
//  Created by 严明俊 on 16/6/29.
//  Copyright © 2016年 yanmingjun. All rights reserved.
//

#include <stdio.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include "IRenderingEngine.hpp"
#define STRINGIFY(A) #A
#include "Simple.vert"
#include "Simple.frag"
#include <iostream>
#include <cmath>
#include "Quaternion.hpp"
#include <vector>

using namespace std;

static const float AnimationDuration = 0.25f;

struct Vertex {
    vec3 position;
    vec4 color;
};

struct Animation {
    Quaternion start;
    Quaternion end;
    Quaternion current;
    float elapsed;
    float duration;
};

class RenderingEngine2 : public IRenderingEngine {
public:
    RenderingEngine2();
    virtual void initialize(int width, int height);
    virtual void render() const;
    virtual void updateAnimation(float timeStep);
    virtual void onRotate(DeviceOrientation newOrientation);
private:
    GLuint buildProgram(const char* vShader, const char* fShader) const;
    GLuint buildShader(const char* shaderSource, GLenum shaderType) const;
    vector<Vertex> _cone;
    vector<Vertex> _disk;
    Animation _animation;
    GLuint _simpleProgram;
    GLuint _framebuffer;
    GLuint _colorRenderbuffer;
    GLuint _depthRenderbuffer;
};

struct IRenderingEngine* CreateRenderEngine2() {
    return new RenderingEngine2();
}

RenderingEngine2::RenderingEngine2() {
    glGenRenderbuffers(1, &_colorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    _animation.duration = AnimationDuration;
}

void RenderingEngine2::initialize(int width, int height) {
    
    const float coneRadius = 0.5f;
    const float coneHeight = 1.866f;
    const int coneSlices = 40;
    
    {
        _cone.resize((coneSlices + 1) * 2);
        vector<Vertex>::iterator vertex = _cone.begin();
        const float dtheta = M_PI * 2 / coneSlices;
        for (float theta = 0; vertex != _cone.end(); theta += dtheta) {
            float brightness = abs(sin(theta));
            vec4 color(brightness, brightness, brightness, 1);
            
            vertex->position = vec3(0, 1, 0);
            vertex->color = color;
            vertex++;
            
            vertex->position.x = coneRadius * cos(theta);
            vertex->position.y = 1 - coneHeight;
            vertex->position.z = coneRadius * sin(theta);
            vertex->color = color;
            vertex++;
        }
    }
    
    {
        _disk.resize(coneSlices + 2);
        vector<Vertex>::iterator vertex = _disk.begin();
        vertex->color = vec4(0.75, 0.75, 0.75, 1);
        vertex->position.x = 0;
        vertex->position.y = 1 - coneHeight;
        vertex->position.z = 0;
        const float dtheta = M_PI * 2 / coneSlices;
        for (float theta = 0; vertex != _disk.end(); theta += dtheta) {
            vertex->color = vec4(0.75, 0.75, 0.75, 1);
            vertex->position.x = coneRadius * cos(theta);
            vertex->position.y = 1 - coneHeight;
            vertex->position.z = coneRadius * sin(theta);
            vertex++;
        }
    }
    
    glGenRenderbuffers(1, &_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    glViewport(0, 0, width, height);
    
    glEnable(GL_DEPTH_TEST);
    
    _simpleProgram = buildProgram(SimpleVertexShader, SimpleFragmentShader);
    glUseProgram(_simpleProgram);
    
    GLint projectionUniform = glGetUniformLocation(_simpleProgram, "Projection");
    float maxX = 1.6;
    float maxY = maxX * height / width;
    mat4 projectionMatrix = mat4::Frustum(-maxX, maxX, -maxY, maxY, 5, 10);
    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, projectionMatrix.Pointer());
}

void RenderingEngine2::render() const {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mat4 rotation(_animation.current.ToMatrix());
    mat4 translation = mat4::Translate(0, 0, -7);
    mat4 modelviewMatrix = rotation * translation;
    GLint modelviewUniform = glGetUniformLocation(_simpleProgram, "Modelview");
    glUniformMatrix4fv(modelviewUniform, 1, GL_FALSE, modelviewMatrix.Pointer());
    
    GLuint positionSlot = glGetAttribLocation(_simpleProgram, "Position");
    GLuint colorSlot = glGetAttribLocation(_simpleProgram, "SourceColor");
    glEnableVertexAttribArray(positionSlot);
    glEnableVertexAttribArray(colorSlot);
    GLsizei stride = sizeof(Vertex);
    glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, stride, &_cone[0].position.x);
    glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, &_cone[0].color.x);
    GLsizei vertexCount = (GLsizei)_cone.size();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);
    
    glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, stride, &_disk[0].position.x);
    glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, &_disk[0].color.x);
    vertexCount = (GLsizei)_disk.size();
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
    
    glDisableVertexAttribArray(positionSlot);
    glDisableVertexAttribArray(colorSlot);
}

void RenderingEngine2::updateAnimation(float timeStep) {
    if (_animation.current == _animation.end) {
        return;
    }
    
    _animation.elapsed += timeStep;
    if (_animation.elapsed >= _animation.duration) {
        _animation.current = _animation.end;
    } else {
        float mu = _animation.elapsed / _animation.duration;
        _animation.current = _animation.start.Slerp(mu, _animation.end);
    }
}

void RenderingEngine2::onRotate(DeviceOrientation newOrientation) {
    vec3 direction;
    switch (newOrientation) {
        case DeviceOrientationUnknown:
        case DeviceOrientationPortrait:
            direction = vec3(0, 1, 0);
            break;
        case DeviceOrientationPortraitUpsideDown:
            direction = vec3(0, -1, 0);
            break;
        case DeviceOrientationFaceUp:
            direction = vec3(0, 0, 1);
            break;
        case DeviceOrientationFaceDown:
            direction = vec3(0, 0, -1);
            break;
        case DeviceOrientationLandscapeLeft:
            direction = vec3(1, 0, 0);
            break;
        case DeviceOrientationLandscapeRight:
            direction = vec3(-1, 0, 0);
            break;
            
        default:
            break;
    }
    _animation.elapsed = 0;
    _animation.start = _animation.current = _animation.end;
    _animation.end = Quaternion::CreateFromVectors(vec3(0, 1, 0), direction);
}

GLuint RenderingEngine2::buildProgram(const char *vShader, const char *fShader) const {
    GLuint programHandle = glCreateProgram();
    GLuint vertexShader = buildShader(vShader, GL_VERTEX_SHADER);
    GLuint fragmentShader = buildShader(SimpleFragmentShader, GL_FRAGMENT_SHADER);
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram(programHandle);
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        GLchar message[256];
        glGetProgramInfoLog(programHandle, sizeof(message), 0, message);
        std::cout << message << std::endl;
        exit(1);
    }
    return programHandle;
}

GLuint RenderingEngine2::buildShader(const char *shaderSource, GLenum shaderType) const {
    GLuint shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &shaderSource, 0);
    glCompileShader(shaderHandle);
    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
    if (compileSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, messages);
        std::cout << messages << std::endl;
        exit(1);
    }
    return shaderHandle;
}






