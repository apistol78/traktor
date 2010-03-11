#ifndef traktor_render_ProgramResourceDx10_H
#define traktor_render_ProgramResourceDx10_H

#include <map>
#include "Render/Ps3/TypesPs3.h"
#include "Render/Resource/ProgramResource.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup PS3
 */
class ProgramResourcePs3 : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourcePs3();

	virtual ~ProgramResourcePs3();

	virtual bool serialize(ISerializer& s);

private:
	friend class ProgramPs3;
	friend class ProgramCompilerPs3;

	CGCbin* m_vertexShaderBin;
	CGCbin* m_pixelShaderBin;
	std::vector< ProgramScalar > m_vertexScalars;
	std::vector< ProgramScalar > m_pixelScalars;
	std::vector< ProgramSampler > m_vertexSamplers;
	std::vector< ProgramSampler > m_pixelSamplers;
	std::map< std::wstring, uint32_t > m_scalarParameterMap;
	std::map< std::wstring, uint32_t > m_textureParameterMap;
	uint32_t m_scalarParameterDataSize;
	uint32_t m_textureParameterDataSize;
	std::vector< uint8_t > m_inputSignature;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_ProgramResourceDx10_H
