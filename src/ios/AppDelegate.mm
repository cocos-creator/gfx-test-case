//
//  AppDelegate.m
//  ios-test
//
//  Created by minggo on 2019/12/5.
//  Copyright © 2019 minggo. All rights reserved.
//

#import "AppDelegate.h"
#import "ViewController.h"
#import "View.h"
#include "tests/TestBase.h"

using namespace cc;

namespace
{
    WindowInfo g_windowInfo;
}

@interface AppDelegate ()

@end

@implementation AppDelegate

@synthesize window;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.

    CGRect rect = [[UIScreen mainScreen] bounds];
    self.window = [[UIWindow alloc] initWithFrame: rect];
    ViewController* viewController = [[ViewController alloc] init];

    UIView* view = [[View alloc] initWithFrame: rect];
    viewController.view = view;

    [self initWindowInfo: view size:rect.size];
    TestBaseI::setWindowInfo(g_windowInfo);
    TestBaseI::nextTest();

    [self run];

    [self.window setRootViewController:viewController];
    [self.window makeKeyAndVisible];

    return YES;
}

- (void)initWindowInfo:(UIView*)view size:(CGSize)size {
#if CC_USE_VULKAN
    g_windowInfo.windowHandle = [view layer];
#else
    g_windowInfo.windowHandle = view;
#endif
    float scale = view.contentScaleFactor;
    g_windowInfo.screen.x = 0;
    g_windowInfo.screen.y = 0;
    g_windowInfo.screen.width = size.width * scale;
    g_windowInfo.screen.height = size.height * scale;
    g_windowInfo.pixelRatio = scale;
}

- (void)run {
    CADisplayLink* displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(loop:)];
    displayLink.preferredFramesPerSecond = 60;
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

-(void)loop:(id)sender {
    TestBaseI::update();
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    TestBaseI::onTouchEnd();
}


@end
