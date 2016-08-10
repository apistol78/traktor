#ifndef traktor_render_GlslProgram_H
#define traktor_render_GlslProgram_H

#include <string>

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class GlslProgram
{
public:
	GlslProgram();

	GlslProgram(
		const std::wstring& vertexShader,
		const std::wstring& fragmentShader
	);

	const std::wstring& getVertexShader() const;

	const std::wstring& getFragmentShader() const;

private:
	std::wstring m_vertexShader;
	std::wstring m_fragmentShader;
};

	}
}

#endif	// traktor_render_GlslProgram_H
