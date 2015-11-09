#ifndef traktor_render_ProgramPs4_H
#define traktor_render_ProgramPs4_H

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

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setFloatParameter(handle_t handle, float param) T_OVERRIDE T_FINAL;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) T_OVERRIDE T_FINAL;
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param) T_OVERRIDE T_FINAL;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) T_OVERRIDE T_FINAL;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) T_OVERRIDE T_FINAL;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) T_OVERRIDE T_FINAL;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) T_OVERRIDE T_FINAL;

	virtual void setStencilReference(uint32_t stencilReference) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_ProgramPs4_H
