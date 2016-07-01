//
//  IRenderingEngine.h
//  HelloCone
//
//  Created by 严明俊 on 16/6/29.
//  Copyright © 2016年 yanmingjun. All rights reserved.
//

#ifndef IRenderingEngine_h
#define IRenderingEngine_h

enum DeviceOrientation {
    DeviceOrientationUnknown,
    DeviceOrientationPortrait,
    DeviceOrientationPortraitUpsideDown,
    DeviceOrientationLandscapeLeft,
    DeviceOrientationLandscapeRight,
    DeviceOrientationFaceUp,
    DeviceOrientationFaceDown,
};

struct IRenderingEngine {
    virtual void initialize(int width, int height) = 0;
    virtual void render() const = 0;
    virtual void updateAnimation(float timeStep) = 0;
    virtual void onRotate(DeviceOrientation newOrientation) = 0;
    virtual ~IRenderingEngine() {}
};

struct IRenderingEngine* CreateRenderEngine1();
struct IRenderingEngine* CreateRenderEngine2();

#endif /* IRenderingEngine_h */
