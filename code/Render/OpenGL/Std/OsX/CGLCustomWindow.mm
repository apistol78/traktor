//
//  CGLCustomWindow.m
//  Traktor MacOSX
//
//  Created by Anders Pistol on 2010-08-04.
//  Copyright 2010 Apple Inc. All rights reserved.
//

#import "Render/OpenGL/Std/OsX/CGLCustomWindow.h"

@implementation CGLCustomWindow

- (BOOL) canBecomeKeyWindow
{
	return YES;
}

- (BOOL) canBecomeMainWindow
{
	return YES;
}

@end
