/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_JitARM_H
#define traktor_render_JitARM_H

#include "Render/Sw/Core/Processor.h"

namespace traktor
{
	namespace render
	{

class JitARM : public Processor
{
public:
	virtual image_t compile(const Program& program);

	virtual void destroy(image_t image);

	virtual void execute(
		const image_t image,
		const Vector4* inUniforms,
		const Vector4* inVaryings,
		const Ref< Sampler >* inSamplers,
		Vector4* outVaryings
	);
};

	}
}

#endif	// traktor_render_JitARM_H
