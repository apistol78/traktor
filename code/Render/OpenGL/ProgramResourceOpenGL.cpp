#include "Render/OpenGL/ProgramResourceOpenGL.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.ProgramResourceOpenGL", ProgramResourceOpenGL, ProgramResource)

ProgramResourceOpenGL::ProgramResourceOpenGL(const ShaderGraph* shaderGraph)
:	m_shaderGraph(const_cast< ShaderGraph* >(shaderGraph))
{
}

bool ProgramResourceOpenGL::serialize(Serializer& s)
{
	if (!ProgramResource::serialize(s))
		return false;

	return s >> MemberRef< ShaderGraph >(L"shaderGraph", m_shaderGraph);
}

	}
}
