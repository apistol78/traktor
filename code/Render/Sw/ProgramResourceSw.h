/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramResourceSw_H
#define traktor_render_ProgramResourceSw_H

#include <map>
#include "Render/Types.h"
#include "Render/Resource/ProgramResource.h"
#include "Render/Sw/RenderStateDesc.h"
#include "Render/Sw/Core/IntrProgram.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup SW
 */
class ProgramResourceSw : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourceSw();

	ProgramResourceSw(
		const IntrProgram& vertexProgram,
		const IntrProgram& pixelProgram,
		const std::map< std::wstring, std::pair< int32_t, int32_t > >& parameterMap,
		const std::map< std::wstring, int32_t >& samplerMap,
		const RenderStateDesc& renderState,
		uint32_t interpolatorCount
	);

	virtual void serialize(ISerializer& s);

	const IntrProgram& getVertexProgram() const { return m_vertexProgram; }

	const IntrProgram& getPixelProgram() const { return m_pixelProgram; }

	const std::map< std::wstring, std::pair< int32_t, int32_t > >& getParameterMap() const { return m_parameterMap; }

	const std::map< std::wstring, int32_t >& getSamplerMap() const { return m_samplerMap; }

	const RenderStateDesc& getRenderState() const { return m_renderState; }

	uint32_t getInterpolatorCount() const { return m_interpolatorCount; }

private:
	IntrProgram m_vertexProgram;
	IntrProgram m_pixelProgram;
	std::map< std::wstring, std::pair< int32_t, int32_t > > m_parameterMap;
	std::map< std::wstring, int32_t > m_samplerMap;
	RenderStateDesc m_renderState;
	uint32_t m_interpolatorCount;
};

	}
}

#endif	// traktor_render_ProgramResourceSw_H
