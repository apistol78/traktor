#include "Render/OpenGL/ES2/ProgramResourceOpenGLES2.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberBuffer : public MemberComplex
{
public:
	MemberBuffer(const std::wstring& name, AutoArrayPtr< uint8_t >& buffer, uint32_t bufferSize)
	:	MemberComplex(name, false)
	,	m_buffer(buffer)
	,	m_bufferSize(bufferSize)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		if (s.getDirection() == ISerializer::SdRead)
			m_buffer.reset(new uint8_t [m_bufferSize]);
		return s >> Member< void* >(getName(), m_buffer.ptr(), m_bufferSize);
	}

private:
	AutoArrayPtr< uint8_t >& m_buffer;
	mutable uint32_t m_bufferSize;
};

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceOpenGLES2", 0, ProgramResourceOpenGLES2, ProgramResource)

ProgramResourceOpenGLES2::ProgramResourceOpenGLES2()
:	m_bufferSize(0)
{
}

ProgramResourceOpenGLES2::ProgramResourceOpenGLES2(
	const void* buffer,
	uint32_t bufferSize,
	const std::vector< SamplerTexture >& samplerTextures,
	const RenderState& renderState
)
:	m_bufferSize(bufferSize)
,	m_samplerTextures(samplerTextures)
,	m_renderState(renderState)
{
	m_buffer.reset(new uint8_t [bufferSize]);
	std::memcpy(m_buffer.ptr(), buffer, bufferSize);
}

bool ProgramResourceOpenGLES2::serialize(ISerializer& s)
{
	uint32_t renderStateSize = sizeof(m_renderState);

	s >> Member< uint32_t >(L"bufferSize", m_bufferSize);
	s >> MemberBuffer(L"buffer", m_buffer, m_bufferSize);
	s >> MemberStlVector< SamplerTexture, MemberSamplerTexture >(L"samplerTextures", m_samplerTextures);
	s >> Member< void* >(L"renderState", &m_renderState, renderStateSize);

	return true;
}

	}
}
