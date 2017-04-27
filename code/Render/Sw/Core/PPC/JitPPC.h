/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_JitPPC_H
#define traktor_render_JitPPC_H

#include "Render/Sw/Core/PPC/JitPPC.h"

namespace traktor
{
	namespace render
	{
	
class JitPPC : public Processor
{
public:
	virtual void* compile(const Program& program);
	
	virtual void execute(
		const Program& program,
		const Vector4* inUniforms,
		const Vector4* inVaryings,
		const Ref< Sampler >* inSamplers,
		Vector4* outVaryings
	);
};
	
	}
}

#endif	// traktor_render_JitPPC_H
