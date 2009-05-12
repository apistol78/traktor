#ifndef traktor_render_ShaderPs3_H
#define traktor_render_ShaderPs3_H

#include <map>
#include <vector>
#include "Render/Shader.h"
#include "Render/Ps3/TypesPs3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ShaderGraph;

class T_DLLCLASS ShaderPs3 : public Shader
{
	T_RTTI_CLASS(ShaderPs3)

public:
	struct Parameter
	{
		std::vector< CGparameter > parameters;
		bool sampler;
		union
		{
			struct
			{
				uint32_t offset;
				uint32_t stride;
			};
			uint32_t stage;
		};
	};

	ShaderPs3();

	virtual ~ShaderPs3();

	bool create(
		const ShaderGraph* shaderGraph,
		const std::wstring& vertexShader,
		const std::wstring& pixelShader,
		const RenderState& renderState
	);

	virtual void destroy();

	virtual void setFloatParameter(const std::wstring& name, float param);

	virtual void setFloatArrayParameter(const std::wstring& name, const float* param, int length);
	
	virtual void setVectorParameter(const std::wstring& name, const Vector4& param);

	virtual void setVectorArrayParameter(const std::wstring& name, const Vector4* param, int length);

	virtual void setMatrixParameter(const std::wstring& name, const Matrix44& param);

	virtual void setMatrixArrayParameter(const std::wstring& name, const Matrix44* param, int length);

	virtual void setSamplerTexture(const std::wstring& name, Texture* texture);

	void bind(RenderMode renderMode);

private:
	char* m_vertexShaderImage;
	char* m_pixelShaderImage;
	char* m_pixelShaderZOnlyImage;

	uint32_t m_vertexShaderUCodeSize;
	void* m_vertexShaderUCode;
	
	uint32_t m_pixelShaderUCodeSize;
	void* m_pixelShaderUCode;
	uint32_t m_pixelShaderOffset;

	uint32_t m_pixelShaderZOnlyUCodeSize;
	void* m_pixelShaderZOnlyUCode;
	uint32_t m_pixelShaderZOnlyOffset;

	std::map< std::wstring, Parameter > m_vertexParameterMap;
	std::vector< float > m_vertexParameters;

	std::map< std::wstring, Parameter > m_pixelParameterMap;
	std::vector< float > m_pixelParameters;

	Texture* m_pixelTextures[16];

	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_ShaderPs3_H
