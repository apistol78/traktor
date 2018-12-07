/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ITextureBinding_H
#define traktor_render_ITextureBinding_H

#include "Render/OpenGL/Std/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class RenderContextOpenGL;

class ITextureBinding
{
public:
	virtual void bindTexture(RenderContextOpenGL* renderContext, uint32_t samplerObject, uint32_t stage) = 0;

	virtual void bindSize(GLint locationSize) = 0;
};
	
	}
}

#endif	// traktor_render_ITextureBinding_H
