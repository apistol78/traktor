#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Capture/ProgramResourceCapture.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceCapture", 0, ProgramResourceCapture, ProgramResource)

void ProgramResourceCapture::serialize(ISerializer& s)
{
	s >> MemberRef< ProgramResource >(L"embedded", m_embedded);
	s >> MemberRefArray< Uniform >(L"uniforms", m_uniforms);
	s >> MemberRefArray< IndexedUniform >(L"indexedUniforms", m_indexedUniforms);
	s >> Member< std::wstring >(L"vertexShader", m_vertexShader);
	s >> Member< std::wstring >(L"pixelShader", m_pixelShader);
}

	}
}
