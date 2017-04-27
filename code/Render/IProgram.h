/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IProgram_H
#define traktor_render_IProgram_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;

/*! \brief Program
 * \ingroup Render
 */
class T_DLLCLASS IProgram : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual void setFloatParameter(handle_t handle, float param) = 0;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) = 0;

	virtual void setVectorParameter(handle_t handle, const Vector4& param) = 0;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) = 0;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) = 0;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) = 0;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) = 0;

	virtual void setStencilReference(uint32_t stencilReference) = 0;

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
};

	}
}

#endif	// traktor_render_IProgram_H
