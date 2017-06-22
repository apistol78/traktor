/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AccGlyphRenderer_H
#define traktor_flash_AccGlyphRenderer_H

#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class RenderContext;
class RenderTargetSet;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace flash
	{
	
class AccGlyphRenderer : public Object
{
	T_RTTI_CLASS;

public:
	bool create(render::IRenderSystem* renderSystem, resource::IResourceManager* resourceManager);

	void destroy();

private:
	Ref< render::RenderTargetSet > m_renderTargetGlyphs;
};
	
	}
}

#endif	// traktor_flash_AccGlyphRenderer_H
