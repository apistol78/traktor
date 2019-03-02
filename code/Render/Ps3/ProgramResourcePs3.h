#pragma once

#include "Core/Containers/SmallMap.h"
#include "Render/Ps3/TypesPs3.h"
#include "Render/Ps3/Blob.h"
#include "Render/Resource/ProgramResource.h"

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

/*!
 * \ingroup PS3
 */
class T_DLLCLASS ProgramResourcePs3 : public ProgramResource
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
	AlignedVector< ProgramScalar > m_vertexScalars;
	AlignedVector< ProgramScalar > m_pixelScalars;
	AlignedVector< ProgramSampler > m_vertexSamplers;
	AlignedVector< ProgramSampler > m_pixelSamplers;
	SmallMap< std::wstring, ScalarParameter > m_scalarParameterMap;
	SmallMap< std::wstring, uint32_t > m_textureParameterMap;
	uint32_t m_scalarParameterDataSize;
	uint32_t m_textureParameterDataSize;
	AlignedVector< uint8_t > m_inputSignature;
	RenderStateGCM m_renderState;
};

	}
}

