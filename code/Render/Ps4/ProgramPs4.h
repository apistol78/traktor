#pragma once

#include "Render/IProgram.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;
class ProgramResourcePs4;

/*!
 * \ingroup GNM
 */
class ProgramPs4 : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramPs4(ContextPs4* context);

	virtual ~ProgramPs4();

	bool create(const ProgramResourcePs4* resource);

	virtual void destroy() override final;

	virtual void setFloatParameter(handle_t handle, float param) override final;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) override final;

	virtual void setVectorParameter(handle_t handle, const Vector4& param) override final;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) override final;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) override final;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) override final;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) override final;

	virtual void setStructBufferParameter(handle_t handle, StructBuffer* structBuffer) override final;

	virtual void setStencilReference(uint32_t stencilReference) override final;
};

	}
}

