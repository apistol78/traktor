#ifndef traktor_render_RenderStateDesc_H
#define traktor_render_RenderStateDesc_H

#include "Render/Nodes.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup SW
 */
struct RenderStateDesc
{
	bool depthEnable;
	bool depthWriteEnable;
	PixelOutput::CullMode cullMode;
	bool blendEnable;
	PixelOutput::BlendOperation blendOperation;
	PixelOutput::BlendFactor blendSource;
	PixelOutput::BlendFactor blendDestination;

	RenderStateDesc()
	:	depthEnable(true)
	,	depthWriteEnable(true)
	,	cullMode(PixelOutput::CmNever)
	,	blendEnable(false)
	,	blendOperation(PixelOutput::BoAdd)
	,	blendSource(PixelOutput::BfOne)
	,	blendDestination(PixelOutput::BfZero)
	{
	}
};

	}
}

#endif	// traktor_render_RenderStateDesc_H
