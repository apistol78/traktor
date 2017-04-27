/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Ps4/ProgramPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramPs4", ProgramPs4, IProgram)

ProgramPs4::ProgramPs4(ContextPs4* context)
{
}

ProgramPs4::~ProgramPs4()
{
	destroy();
}

bool ProgramPs4::create(const ProgramResourcePs4* resource)
{
	return true;
}

void ProgramPs4::destroy()
{
}

void ProgramPs4::setFloatParameter(handle_t handle, float param)
{
	setFloatArrayParameter(handle, &param, 1);
}

void ProgramPs4::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
}

void ProgramPs4::setVectorParameter(handle_t handle, const Vector4& param)
{
	setVectorArrayParameter(handle, &param, 1);
}

void ProgramPs4::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
}

void ProgramPs4::setMatrixParameter(handle_t handle, const Matrix44& param)
{
}

void ProgramPs4::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
}

void ProgramPs4::setTextureParameter(handle_t handle, ITexture* texture)
{
}

void ProgramPs4::setStencilReference(uint32_t stencilReference)
{
}

	}
}
