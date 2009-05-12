#include "Spray/Emitter.h"
#include "Spray/EmitterInstance.h"
#include "Spray/Source.h"
#include "Spray/Modifier.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.spray.Emitter", Emitter, Serializable)

Emitter::Emitter()
:	m_middleAge(0.2f)
,	m_warmUp(0.0f)
{
}

EmitterInstance* Emitter::createInstance()
{
	return gc_new< EmitterInstance >(this);
}

int Emitter::getVersion() const
{
	return 1;
}

bool Emitter::serialize(Serializer& s)
{
	s >> MemberRef< Source >(L"source", m_source);
	s >> MemberRefArray< Modifier >(L"modifiers", m_modifiers);
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> Member< float >(L"middleAge", m_middleAge);
	if (s.getVersion() >= 1)
		s >> Member< float >(L"warmUp", m_warmUp);
	return true;
}

	}
}
