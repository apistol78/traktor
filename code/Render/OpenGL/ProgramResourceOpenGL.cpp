#include "Render/OpenGL/ProgramResourceOpenGL.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberSamplerTexture : public MemberComplex
{
public:
	MemberSamplerTexture(const std::wstring& name, SamplerTexture& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< std::wstring >(L"sampler", m_ref.sampler);
		s >> Member< std::wstring >(L"texture", m_ref.texture);
		return true;
	}

private:
	SamplerTexture& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceOpenGL", 0, ProgramResourceOpenGL, ProgramResource)

ProgramResourceOpenGL::ProgramResourceOpenGL()
{
}

ProgramResourceOpenGL::ProgramResourceOpenGL(
	const std::wstring& vertexShader,
	const std::wstring& fragmentShader,
	const std::vector< SamplerTexture >& samplerTextures,
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
	uint32_t renderStateSize = sizeof(m_renderState);

	s >> Member< std::wstring >(L"vertexShader", m_vertexShader);
	s >> Member< std::wstring >(L"fragmentShader", m_fragmentShader);
	s >> MemberStlVector< SamplerTexture, MemberSamplerTexture >(L"samplerTextures", m_samplerTextures);
	s >> Member< void* >(L"renderState", &m_renderState, renderStateSize);

	return true;
}

	}
}
