/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Spray/Editor/BoxSourceRenderer.h"
#include "Spray/Sources/BoxSourceData.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.BoxSourceRenderer", BoxSourceRenderer, SourceRenderer)

void BoxSourceRenderer::render(render::PrimitiveRenderer* primitiveRenderer, const SourceData* sourceData) const
{
	const BoxSourceData* boxSource = checked_type_cast< const BoxSourceData* >(sourceData);
	primitiveRenderer->drawWireAabb(boxSource->m_position, boxSource->m_extent, Color4ub(255, 255, 0));
}

	}
}
