/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ITextureBinding_H
#define traktor_render_ITextureBinding_H

#include "Render/OpenGL/ES2/TypesOpenGLES2.h"

namespace traktor
{
	namespace render
	{

class ITextureBinding
{
public:
	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture) = 0;

	virtual void bindSize(GLint locationSize) = 0;
};
	
	}
}

#endif	// traktor_render_ITextureBinding_H
