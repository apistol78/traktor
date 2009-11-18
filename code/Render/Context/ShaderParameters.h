#ifndef traktor_render_ShaderParameters_H
#define traktor_render_ShaderParameters_H

#include <string>
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Render/Shader.h"

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

class Shader;
class ITexture;
class RenderContext;

/*! \brief Shader parameter values.
 * \ingroup Render
 */
class T_DLLCLASS ShaderParameters
{
public:
	ShaderParameters();

	void setTechnique(handle_t handle);

	/*! \name Record parameters.
	 *
	 * Must set all parameters between begin- and endParameters.
	 */
	//@{

	void beginParameters(RenderContext* context);

	void endParameters(RenderContext* context);

	void setBooleanParameter(handle_t handle, bool param);

	void setFloatParameter(handle_t handle, float param);

	void setFloatArrayParameter(handle_t handle, const float* param, int length);
	
	void setVectorParameter(handle_t handle, const Vector4& param);

	void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	void setMatrixParameter(handle_t handle, const Matrix44& param);

	void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	void setSamplerTexture(handle_t handle, ITexture* texture);

	void fixup(Shader* shader) const;

	//@}

	/*! \name Set parameter by name.
	 *
	 * These methods are implemented for backward compatibility and should not be used in
	 * time critical paths.
	 */
	//@{

	inline void setTechnique(const std::wstring& name) { setTechnique(getParameterHandle(name)); }

	inline void setBooleanParameter(const std::wstring& name, bool param) { setBooleanParameter(getParameterHandle(name), param); }

	inline void setFloatParameter(const std::wstring& name, float param) { setFloatParameter(getParameterHandle(name), param); }

	inline void setFloatArrayParameter(const std::wstring& name, const float* param, int length) { setFloatArrayParameter(getParameterHandle(name), param, length); }

	inline void setVectorParameter(const std::wstring& name, const Vector4& param) { setVectorParameter(getParameterHandle(name), param); }

	inline void setVectorArrayParameter(const std::wstring& name, const Vector4* param, int length) { setVectorArrayParameter(getParameterHandle(name), param, length); }

	inline void setMatrixParameter(const std::wstring& name, const Matrix44& param) { setMatrixParameter(getParameterHandle(name), param); }

	inline void setMatrixArrayParameter(const std::wstring& name, const Matrix44* param, int length) { setMatrixArrayParameter(getParameterHandle(name), param, length); }

	inline void setSamplerTexture(const std::wstring& name, ITexture* texture) { setSamplerTexture(getParameterHandle(name), texture); }

	//@}

private:
	handle_t m_technique;
	uint8_t* m_parameterFirst;
	uint8_t* m_parameterLast;
};

	}
}

#endif	// traktor_render_ShaderParameters_H
