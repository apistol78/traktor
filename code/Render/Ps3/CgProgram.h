#ifndef traktor_render_CgProgram_H
#define traktor_render_CgProgram_H

#include "Core/Config.h"

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

#endif	// traktor_render_CgProgram_H
