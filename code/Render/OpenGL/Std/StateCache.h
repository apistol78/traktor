/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_StateCache_H
#define traktor_render_StateCache_H

#include "Core/Object.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class StateCache : public Object
{
	T_RTTI_CLASS;
	
public:
	StateCache();

	void reset();
	
	void setRenderState(const RenderStateOpenGL& renderState);

private:
	RenderStateOpenGL m_renderState;
};
	
	}
}

#endif	// traktor_render_StateCache_H
