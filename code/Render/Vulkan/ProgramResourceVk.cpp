#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Vulkan/ProgramResourceVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceVk", 0, ProgramResourceVk, ProgramResource)

ProgramResourceVk::ProgramResourceVk()
{
}

void ProgramResourceVk::serialize(ISerializer& s)
{
	s >> MemberStlVector< uint32_t >(L"vertexShader", m_vertexShader);
	s >> MemberStlVector< uint32_t >(L"fragmentShader", m_fragmentShader);
}

	}
}
