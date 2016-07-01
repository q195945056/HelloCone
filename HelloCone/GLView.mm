//
//  GLView.m
//  HelloCone
//
//  Created by 严明俊 on 16/6/29.
//  Copyright © 2016年 yanmingjun. All rights reserved.
//

#import "GLView.h"
#import "IRenderingEngine.hpp"
#import <OpenGLES/ES2/gl.h>

static const BOOL ForceES1 = NO;

@interface GLView ()

@property (nonatomic, strong) EAGLContext *context;
@property (nonatomic) NSTimeInterval timestamp;

@end

@implementation GLView {
    IRenderingEngine *_renderingEngine;
}

+ (Class)layerClass {
    return [CAEAGLLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
        _context = [[EAGLContext alloc] initWithAPI:api];
        if (!_context || ForceES1) {
            api = kEAGLRenderingAPIOpenGLES1;
            _context = [[EAGLContext alloc] initWithAPI:api];
        }
        if (!_context || ![EAGLContext setCurrentContext:_context]) {
            return nil;
        }
        
        if (api == kEAGLRenderingAPIOpenGLES1) {
            _renderingEngine = CreateRenderEngine1();
        } else {
            _renderingEngine = CreateRenderEngine2();
        }
        
        CAEAGLLayer *glLayer = (CAEAGLLayer *)self.layer;
        [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:glLayer];
        _renderingEngine->initialize(frame.size.width, frame.size.height);
        [self drawView:nil];
        
        _timestamp = CACurrentMediaTime();
        CADisplayLink *displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onRotate:) name:UIDeviceOrientationDidChangeNotification object:nil];
    }
    return self;
}

- (void)drawView:(CADisplayLink *)displayLink {
    if (displayLink) {
        NSTimeInterval elapsed = displayLink.timestamp - self.timestamp;
        self.timestamp = displayLink.timestamp;
        _renderingEngine->updateAnimation(elapsed);
    }
    _renderingEngine->render();
    [self.context presentRenderbuffer:GL_RENDERBUFFER];
}

- (void)onRotate:(NSNotification *)notification {
    UIDeviceOrientation orientaion = [[UIDevice currentDevice] orientation];
    _renderingEngine->onRotate((DeviceOrientation)orientaion);
}
                                      

@end
