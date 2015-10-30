#ifndef traktor_render_ProgramResourcePs3_H
#define traktor_render_ProgramResourcePs3_H

#include <map>
#include "Render/Ps3/TypesPs3.h"
#include "Render/Ps3/Blob.h"
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

	virtual void serialize(ISerializer& s);

private:
	friend class ProgramPs3;
	friend class ProgramCompilerPs3;

	Blob m_vertexShaderBin;
	Blob m_pixelShaderBin;
	std::vector< ProgramScalar > m_vertexScalars;
	std::vector< ProgramScalar > m_pixelScalars;
	std::vector< ProgramSampler > m_vertexSamplers;
	std::vector< ProgramSampler > m_pixelSamplers;
	std::map< std::wstring, ScalarParameter > m_scalarParameterMap;
	std::map< std::wstring, uint32_t > m_textureParameterMap;
	uint32_t m_scalarParameterDataSize;
	uint32_t m_textureParameterDataSize;
	std::vector< uint8_t > m_inputSignature;
	RenderStateGCM m_renderState;
};

	}
}

#endif	// traktor_render_ProgramResourcePs3_H
