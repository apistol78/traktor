#include "Render/Sw/ProgramResourceSw.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.ProgramResourceSw", ProgramResourceSw, ProgramResource)

ProgramResourceSw::ProgramResourceSw(const ShaderGraph* shaderGraph)
:	m_shaderGraph(const_cast< ShaderGraph* >(shaderGraph))
{
}

bool ProgramResourceSw::serialize(Serializer& s)
{
	return s >> MemberRef< ShaderGraph >(L"shaderGraph", m_shaderGraph);
}

	}
}
