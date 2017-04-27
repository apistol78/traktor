/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if defined(WINCE)
#include <windows.h>
#endif
#include "Render/Sw/Core/ARM/JitARM.h"
#include "Render/Sw/Core/ARM/AssemblerARM.h"
#include "Render/Sw/Core/Program.h"
#include "Render/Sw/Core/Sampler.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct InternalImage
{
	float registers[256 * 4];
	float constants[256 * 4];
	void* native;
};

		}

Processor::image_t JitARM::compile(const Program& program)
{
	return 0;
}

void JitARM::destroy(image_t image)
{
}

void JitARM::execute(
	const image_t image,
	const Vector4* inUniforms,
	const Vector4* inVaryings,
	const Ref< Sampler >* inSamplers,
	Vector4* outVaryings
)
{
}

	}
}
