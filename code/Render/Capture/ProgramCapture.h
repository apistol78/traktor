#ifndef traktor_render_ProgramCapture_H
#define traktor_render_ProgramCapture_H

#include "Render/IProgram.h"

namespace traktor
{
	namespace render
	{

class ProgramCapture : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramCapture(IProgram* program, const wchar_t* const tag);

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);

	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setTextureParameter(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

private:
	friend class RenderViewCapture;

	Ref< IProgram > m_program;
	std::wstring m_tag;
};

	}
}

#endif	// traktor_render_ProgramCapture_H
