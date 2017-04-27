/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderStateDesc_H
#define traktor_render_RenderStateDesc_H

#include "Render/Shader/Nodes.h"

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
	CullMode cullMode;
	bool blendEnable;
	BlendOperation blendOperation;
	BlendFactor blendSource;
	BlendFactor blendDestination;

	RenderStateDesc()
	:	depthEnable(true)
	,	depthWriteEnable(true)
	,	cullMode(CmNever)
	,	blendEnable(false)
	,	blendOperation(BoAdd)
	,	blendSource(BfOne)
	,	blendDestination(BfZero)
	{
	}
};

	}
}

#endif	// traktor_render_RenderStateDesc_H
