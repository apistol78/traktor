/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Sw/Core/PPC/JitPPC.h"

namespace traktor
{
	namespace render
	{

void* JitPPC::compile(const Program& program)
{
	return 0;
}
	
void JitPPC::execute(
	const Program& program,
	const Vector4* inUniforms,
	const Vector4* inVaryings,
	const Ref< Sampler >* inSamplers,
	Vector4* outVaryings
)
{
}
	
	}
}
