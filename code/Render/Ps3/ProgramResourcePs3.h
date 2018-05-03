/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramResourcePs3_H
#define traktor_render_ProgramResourcePs3_H

#include "Core/Containers/SmallMap.h"
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

#endif	// traktor_render_ProgramResourcePs3_H
