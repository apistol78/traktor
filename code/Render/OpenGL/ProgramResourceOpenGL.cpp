#include "Render/OpenGL/ProgramResourceOpenGL.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceOpenGL", 1, ProgramResourceOpenGL, ProgramResource)

ProgramResourceOpenGL::ProgramResourceOpenGL()
{
}

ProgramResourceOpenGL::ProgramResourceOpenGL(
	const std::wstring& vertexShader,
	const std::wstring& fragmentShader,
	const std::map< std::wstring, int32_t >& samplerTextures,
	const RenderState& renderState
)
:	m_vertexShader(vertexShader)
,	m_fragmentShader(fragmentShader)
,	m_samplerTextures(samplerTextures)
,	m_renderState(renderState)
{
}

bool ProgramResourceOpenGL::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 1);

	uint32_t renderStateSize = sizeof(m_renderState);

	s >> Member< std::wstring >(L"vertexShader", m_vertexShader);
	s >> Member< std::wstring >(L"fragmentShader", m_fragmentShader);
	s >> MemberStlMap< std::wstring, int32_t >(L"samplerTextures", m_samplerTextures);
	s >> Member< void* >(L"renderState", &m_renderState, renderStateSize);

	return true;
}

	}
}
