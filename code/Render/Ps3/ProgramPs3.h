#ifndef traktor_render_ProgramPs3_H
#define traktor_render_ProgramPs3_H

#include <map>
#include <vector>
#include "Render/IProgram.h"
#include "Render/Ps3/TypesPs3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class LocalMemoryObject;
class ProgramResourcePs3;
class StateCachePs3;

class T_DLLCLASS ProgramPs3 : public IProgram
{
	T_RTTI_CLASS;

public:
	struct Parameter
	{
		std::vector< CGparameter > parameters;
		uint32_t offset;
		uint32_t stride;
	};

	struct Sampler
	{
		uint32_t stage;
	};

	ProgramPs3();

	virtual ~ProgramPs3();

	bool create(const ProgramResourcePs3* resource);

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);

	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setSamplerTexture(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

	void bind(StateCachePs3& stateCache);

private:
	static ProgramPs3* ms_activeProgram;
	Ref< const ProgramResourcePs3 > m_resource;
	CGprogram m_vertexProgram;
	CGprogram m_pixelProgram;
	LocalMemoryObject* m_vertexShaderUCode;
	LocalMemoryObject* m_pixelShaderUCode;
	std::map< handle_t, Parameter > m_vertexParameterMap;
	std::vector< float > m_vertexParameters;
	std::map< handle_t, Parameter > m_pixelParameterMap;
	std::vector< float > m_pixelParameters;
	std::map< handle_t, Sampler > m_pixelSamplerMap;
	ITexture* m_pixelTextures[8];
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_ProgramPs3_H
