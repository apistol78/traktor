#include "Render/Sw/ProgramResourceSw.h"
#include "Render/Shader/ShaderGraph.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceSw", 0, ProgramResourceSw, ProgramResource)

ProgramResourceSw::ProgramResourceSw(const ShaderGraph* shaderGraph)
:	m_shaderGraph(const_cast< ShaderGraph* >(shaderGraph))
{
}

bool ProgramResourceSw::serialize(ISerializer& s)
{
	return s >> MemberRef< ShaderGraph >(L"shaderGraph", m_shaderGraph);
}

	}
}
