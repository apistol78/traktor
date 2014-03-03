#include "Core/Misc/SafeDestroy.h"
#include "Render/Capture/ProgramCapture.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramCapture", ProgramCapture, IProgram)

ProgramCapture::ProgramCapture(IProgram* program, const wchar_t* const tag)
:	m_program(program)
,	m_tag(tag)
{
}

void ProgramCapture::destroy()
{
	safeDestroy(m_program);
}

void ProgramCapture::setFloatParameter(handle_t handle, float param)
{
	m_program->setFloatParameter(handle, param);
}

void ProgramCapture::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	m_program->setFloatArrayParameter(handle, param, length);
}

void ProgramCapture::setVectorParameter(handle_t handle, const Vector4& param)
{
	m_program->setVectorParameter(handle, param);
}

void ProgramCapture::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	m_program->setVectorArrayParameter(handle, param, length);
}

void ProgramCapture::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	m_program->setMatrixParameter(handle, param);
}

void ProgramCapture::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	m_program->setMatrixArrayParameter(handle, param, length);
}

void ProgramCapture::setTextureParameter(handle_t handle, ITexture* texture)
{
	m_program->setTextureParameter(handle, texture);
}

void ProgramCapture::setStencilReference(uint32_t stencilReference)
{
	m_program->setStencilReference(stencilReference);
}

	}
}
