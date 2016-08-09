#include "Render/Vulkan/ProgramVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramVk", ProgramVk, IProgram)

ProgramVk::ProgramVk()
{
}

ProgramVk::~ProgramVk()
{
	destroy();
}

bool ProgramVk::create(const ProgramResourceVk* resource)
{
	return true;
}

void ProgramVk::destroy()
{
}

void ProgramVk::setFloatParameter(handle_t handle, float param)
{
	setFloatArrayParameter(handle, &param, 1);
}

void ProgramVk::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
}

void ProgramVk::setVectorParameter(handle_t handle, const Vector4& param)
{
	setVectorArrayParameter(handle, &param, 1);
}

void ProgramVk::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
}

void ProgramVk::setMatrixParameter(handle_t handle, const Matrix44& param)
{
}

void ProgramVk::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
}

void ProgramVk::setTextureParameter(handle_t handle, ITexture* texture)
{
}

void ProgramVk::setStencilReference(uint32_t stencilReference)
{
}

	}
}
