/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_CgProgram_H
#define traktor_render_CgProgram_H

#include <map>
#include "Render/Ps3/TypesPs3.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup PS3
 */
class CgProgram
{
public:
	CgProgram();

	CgProgram(
		const std::wstring& vertexShader,
		const std::wstring& pixelShader,
		const std::map< std::wstring, int32_t >& vertexTextures,
		const std::map< std::wstring, int32_t >& pixelTextures,
		const RenderStateGCM& renderState,
		uint32_t registerCount
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getPixelShader() const;

	const std::map< std::wstring, int32_t >& getVertexTextures() const;

	const std::map< std::wstring, int32_t >& getPixelTextures() const;

	const RenderStateGCM& getRenderState() const;

	uint32_t getRegisterCount() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	std::map< std::wstring, int32_t > m_vertexTextures;
	std::map< std::wstring, int32_t > m_pixelTextures;
	RenderStateGCM m_renderState;
	uint32_t m_registerCount;
};

	}
}

#endif	// traktor_render_CgProgram_H
