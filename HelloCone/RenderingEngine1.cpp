//
//  RenderingEngine1.cpp
//  HelloCone
//
//  Created by 严明俊 on 16/6/29.
//  Copyright © 2016年 yanmingjun. All rights reserved.
//

#include <stdio.h>
#include "IRenderingEngine.hpp"
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <vector>
#include "Quaternion.hpp"

static const float AnimationDuration = 0.25f;
using namespace std;

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

class RenderingEngine1 : public IRenderingEngine {
public:
    RenderingEngine1();
    virtual void initialize(int width, int height);
    virtual void render() const;
    virtual void updateAnimation(float timeStep);
    virtual void onRotate(DeviceOrientation newOrientation);
private:
    vector<Vertex> _cone;
    vector<Vertex> _disk;
    Animation _animation;
    GLuint _framebuffer;
    GLuint _colorRenderbuffer;
    GLuint _depthRenderbuffer;
};

struct IRenderingEngine* CreateRenderEngine1() {
    return new RenderingEngine1();
}

RenderingEngine1::RenderingEngine1() {
    glGenRenderbuffersOES(1, &_colorRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, _colorRenderbuffer);
    _animation.duration = AnimationDuration;
}

void RenderingEngine1::initialize(int width, int height) {
    
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
    
    glGenRenderbuffersOES(1, &_depthRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, _depthRenderbuffer);
    glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, width, height);
    
    glGenFramebuffersOES(1, &_framebuffer);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, _framebuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, _colorRenderbuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _depthRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, _colorRenderbuffer);
    
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    float maxX = 1.6;
    float maxY = maxX * height / width;
    glMatrixMode(GL_PROJECTION);
    glFrustumf(-maxX, maxX, -maxY, maxY, 5, 10);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(0, 0, -7);
}

void RenderingEngine1::render() const {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    mat4 rotation(_animation.current.ToMatrix());
    glMultMatrixf(rotation.Pointer());
    GLsizei stride = sizeof(Vertex);
    glVertexPointer(3, GL_FLOAT, stride, &_cone[0].position.x);
    glColorPointer(4, GL_FLOAT, stride, &_cone[0].color.x);
    GLsizei vertexCount = (GLsizei)_cone.size();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);
    
    glVertexPointer(3, GL_FLOAT, stride, &_disk[0].position.x);
    glVertexPointer(4, GL_FLOAT, stride, &_disk[0].color.x);
    vertexCount = (GLsizei)_disk.size();
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glPopMatrix();
}

void RenderingEngine1::updateAnimation(float timeStep) {
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

void RenderingEngine1::onRotate(DeviceOrientation newOrientation) {
    vec3 direction;
    switch (newOrientation) {
        case DeviceOrientationUnknown:
        case DeviceOrientationPortrait:
            direction = vec3(0, 1, 0);
            break;
        case DeviceOrientationPortraitUpsideDown:
            direction = vec3(0, -1, 0);
            break;
        case DeviceOrientationFaceDown:
            direction = vec3(0, 0, -1);
            break;
        case DeviceOrientationFaceUp:
            direction = vec3(0, 0, 1);
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
