/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramParameters_H
#define traktor_render_ProgramParameters_H

#include <string>
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
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
	namespace render
	{

class IProgram;
class ITexture;
class RenderContext;

/*! \brief Program parameter values.
 * \ingroup Render
 */
class T_DLLCLASS ProgramParameters
{
public:
	ProgramParameters();

	void attachParameters(ProgramParameters* attachParameters);

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

	void setStencilReference(uint32_t stencilReference);

	void fixup(IProgram* program) const;

	//@}

	/*! \name Set parameter by name.
	 *
	 * These methods are implemented for backward compatibility and should not be used in
	 * time critical paths.
	 */
	//@{

	inline void setFloatParameter(const std::wstring& name, float param) { setFloatParameter(getParameterHandle(name), param); }

	inline void setFloatArrayParameter(const std::wstring& name, const float* param, int length) { setFloatArrayParameter(getParameterHandle(name), param, length); }

	inline void setVectorParameter(const std::wstring& name, const Vector4& param) { setVectorParameter(getParameterHandle(name), param); }

	inline void setVectorArrayParameter(const std::wstring& name, const Vector4* param, int length) { setVectorArrayParameter(getParameterHandle(name), param, length); }

	inline void setMatrixParameter(const std::wstring& name, const Matrix44& param) { setMatrixParameter(getParameterHandle(name), param); }

	inline void setMatrixArrayParameter(const std::wstring& name, const Matrix44* param, int length) { setMatrixArrayParameter(getParameterHandle(name), param, length); }

	inline void setTextureParameter(const std::wstring& name, ITexture* texture) { setTextureParameter(getParameterHandle(name), texture); }

	//@}

private:
	ProgramParameters* m_attachParameters;
	uint8_t* m_parameterFirst;
	uint8_t* m_parameterLast;
};

	}
}

#endif	// traktor_render_ProgramParameters_H
