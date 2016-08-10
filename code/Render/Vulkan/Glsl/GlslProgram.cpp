#include "Render/Vulkan/Glsl/GlslProgram.h"

namespace traktor
{
	namespace render
	{

GlslProgram::GlslProgram()
{
}

GlslProgram::GlslProgram(
	const std::wstring& vertexShader,
	const std::wstring& fragmentShader
)
:	m_vertexShader(vertexShader)
,	m_fragmentShader(fragmentShader)
{
}

const std::wstring& GlslProgram::getVertexShader() const
{
	return m_vertexShader;
}

const std::wstring& GlslProgram::getFragmentShader() const
{
	return m_fragmentShader;
}

	}
}
