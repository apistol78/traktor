#include "Render/OpenGL/ProgramResourceOpenGL.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceOpenGL", 0, ProgramResourceOpenGL, ProgramResource)

ProgramResourceOpenGL::ProgramResourceOpenGL()
{
}

ProgramResourceOpenGL::ProgramResourceOpenGL(
	const std::wstring& vertexShader,
	const std::wstring& fragmentShader,
	const std::set< std::wstring >& vertexSamplers,
	const std::set< std::wstring >& fragmentSamplers,
	const RenderState& renderState
)
:	m_vertexShader(vertexShader)
,	m_fragmentShader(fragmentShader)
,	m_vertexSamplers(vertexSamplers)
,	m_fragmentSamplers(fragmentSamplers)
,	m_renderState(renderState)
{
}

bool ProgramResourceOpenGL::serialize(ISerializer& s)
{
	uint32_t renderStateSize = sizeof(m_renderState);

	s >> Member< std::wstring >(L"vertexShader", m_vertexShader);
	s >> Member< std::wstring >(L"fragmentShader", m_fragmentShader);
	s >> MemberStlSet< std::wstring >(L"vertexSamplers", m_vertexSamplers);
	s >> MemberStlSet< std::wstring >(L"fragmentSamplers", m_fragmentSamplers);
	s >> Member< void* >(L"renderState", &m_renderState, renderStateSize);

	return true;
}

	}
}
