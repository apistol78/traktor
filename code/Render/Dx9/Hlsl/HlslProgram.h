/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_HlslProgram_H
#define traktor_render_HlslProgram_H

#include "Render/Dx9/StateBlockDx9.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX9 Xbox360
 */
class HlslProgram
{
public:
	HlslProgram();

	HlslProgram(
		const std::wstring& vertexShader,
		const std::wstring& pixelShader,
		const std::set< std::wstring >& vertexTextures,
		const std::map< uint32_t, std::pair< std::wstring, int32_t > >& vertexSamplers,
		const std::set< std::wstring >& pixelTextures,
		const std::map< uint32_t, std::pair< std::wstring, int32_t > >& pixelSamplers,
		const StateBlockDx9& state
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getPixelShader() const;

	const std::set< std::wstring >& getVertexTextures() const;

	const std::map< uint32_t, std::pair< std::wstring, int32_t > >& getVertexSamplers() const;

	const std::set< std::wstring >& getPixelTextures() const;

	const std::map< uint32_t, std::pair< std::wstring, int32_t > >& getPixelSamplers() const;

	const StateBlockDx9& getState() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
	std::set< std::wstring > m_vertexTextures;
	std::map< uint32_t, std::pair< std::wstring, int32_t > > m_vertexSamplers;
	std::set< std::wstring > m_pixelTextures;
	std::map< uint32_t, std::pair< std::wstring, int32_t > > m_pixelSamplers;
	StateBlockDx9 m_state;
};

	}
}

#endif	// traktor_render_HlslProgram_H
