/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "Core/Assert.h"
#include "Core/Log/Log.h"
#include "Render/Vulkan/macOS/Metal.h"

@interface MetalDelegate : NSObject< CALayerDelegate >
{
    id m_hack;
    NSView* m_view;
}

- (id) initWithView:(NSView*)view;

- (void) drawLayer:(CALayer *)layer inContext:(CGContextRef)ctx;

- (void) displayLayer:(CALayer *)layer;

@end

@implementation MetalDelegate

- (id) initWithView: (NSView*)view
{
    m_view = view;
    m_hack = self;  // !!HACK!! Ensure delegate isn't released by creating a ref cycle...
	return self;
}

- (void) drawLayer:(CALayer *)layer inContext:(CGContextRef)ctx
{
    NSRect rect = [m_view bounds];
    rect.origin.x =
    rect.origin.y = 0;
    [m_view drawRect:rect];
}

- (void) displayLayer:(CALayer *)layer
{
    NSRect rect = [m_view bounds];
    rect.origin.x =
    rect.origin.y = 0;
    [m_view drawRect:rect];
}

@end

namespace traktor
{
    namespace render
    {

void attachMetalLayer(void* view_)
{
    NSView* view = (__bridge NSView*)view_;
    T_FATAL_ASSERT([view isKindOfClass:[NSView class]]);

    if (![view.layer isKindOfClass:[CAMetalLayer class]])
    {
        CALayer* layer = [CAMetalLayer layer];
        [view setLayer:layer];

        view.layerContentsRedrawPolicy = NSViewLayerContentsRedrawOnSetNeedsDisplay;        
        [view setWantsLayer:YES];
   
        auto delegate = [[MetalDelegate alloc] initWithView:view];
        layer.delegate = delegate;
    } 
}

void* getMetalLayer(void* view_)
{
    NSView* view = (__bridge NSView*)view_;
    CALayer* layer = (CALayer*)view.layer;
    return (__bridge void*)layer;
}

    }
}