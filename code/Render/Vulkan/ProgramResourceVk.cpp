#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Vulkan/ProgramResourceVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceVk", 0, ProgramResourceVk, ProgramResource)

ProgramResourceVk::ProgramResourceVk()
:	m_parameterScalarSize(0)
,	m_parameterTextureSize(0)
{
}

void ProgramResourceVk::serialize(ISerializer& s)
{
	s >> MemberStlVector< uint32_t >(L"vertexShader", m_vertexShader);
	s >> MemberStlVector< uint32_t >(L"fragmentShader", m_fragmentShader);
	s >> MemberStaticArray< UniformBufferDesc, 3, MemberComposite< UniformBufferDesc > >(L"vertexUniformBuffers", m_vertexUniformBuffers);
	s >> MemberStaticArray< UniformBufferDesc, 3, MemberComposite< UniformBufferDesc > >(L"fragmentUniformBuffers", m_fragmentUniformBuffers);
	s >> MemberStlVector< ParameterDesc, MemberComposite< ParameterDesc > >(L"parameters", m_parameters);
	s >> Member< uint32_t >(L"parameterScalarSize", m_parameterScalarSize);
	s >> Member< uint32_t >(L"parameterTextureSize", m_parameterTextureSize);
}

void ProgramResourceVk::ParameterDesc::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< uint32_t >(L"offset", offset);
	s >> Member< uint32_t >(L"size", size);
}

void ProgramResourceVk::ParameterMappingDesc::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"uniformBufferOffset", uniformBufferOffset);
	s >> Member< uint32_t >(L"offset", offset);
	s >> Member< uint32_t >(L"size", size);
}

void ProgramResourceVk::UniformBufferDesc::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"size", size);
	s >> MemberStlVector< ParameterMappingDesc, MemberComposite< ParameterMappingDesc > >(L"parameters", parameters);
}

	}
}
