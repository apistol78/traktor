#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "Core/Assert.h"
#include "Core/Log/Log.h"
#include "Render/Vulkan/macOS/Metal.h"

@interface MetalDelegate : NSObject< CALayerDelegate >
{
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
    NSView* view = (NSView*)view_;
    T_FATAL_ASSERT([view isKindOfClass:[NSView class]]);
    if (![view.layer isKindOfClass:[CAMetalLayer class]])
    {
        CALayer* layer = [CAMetalLayer layer];
        [view setLayer:layer];

        view.layerContentsRedrawPolicy = NSViewLayerContentsRedrawOnSetNeedsDisplay;        
        [view setWantsLayer:YES];

        layer.delegate = [[MetalDelegate alloc] initWithView:view];
    } 
}

    }
}