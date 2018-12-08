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

class ITextureBinding
{
public:
	virtual void bindTexture() const = 0;

	virtual void bindSize(GLint locationSize) const = 0;

	virtual bool haveMips() const = 0;
};
	
	}
}

#endif	// traktor_render_ITextureBinding_H
