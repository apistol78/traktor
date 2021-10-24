#pragma once

#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Vector4;
class Matrix44;

	namespace render
	{

class IBufferView;
class IProgram;
class ITexture;
class RenderContext;

/*! Program parameter values.
 * \ingroup Render
 */
class T_DLLCLASS ProgramParameters
{
public:
	/*! \name Record parameters.
	 *
	 * Must set all parameters between begin- and endParameters.
	 */
	//@{

	void beginParameters(RenderContext* context);

	void endParameters(RenderContext* context);

	void setFloatParameter(handle_t handle, float param);

	void setFloatArrayParameter(handle_t handle, const float* param, int length);

	void setVectorParameter(handle_t handle, const Vector4& param);

	void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	void setMatrixParameter(handle_t handle, const Matrix44& param);

	void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	void setTextureParameter(handle_t handle, ITexture* texture);

	void setImageViewParameter(handle_t handle, ITexture* imageView);

	void setBufferViewParameter(handle_t handle, const IBufferView* bufferView);

	void setStencilReference(uint32_t stencilReference);

	void attachParameters(const ProgramParameters* attachParameters);

	void fixup(IProgram* program) const;

	//@}

private:
	uint8_t* m_parameterFirst = nullptr;
	uint8_t* m_parameterLast = nullptr;
};

	}
}

