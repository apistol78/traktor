/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_PsslProgram_H
#define traktor_render_PsslProgram_H

#include <string>
#include "Core/Config.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup GNM
 */
class PsslProgram
{
public:
	PsslProgram();

	PsslProgram(
		const std::wstring& vertexShader,
		const std::wstring& pixelShader
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getPixelShader() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_pixelShader;
};

	}
}

#endif	// traktor_render_PsslProgram_H
