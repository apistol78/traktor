#include "Spray/Emitter.h"
#include "Spray/EmitterInstance.h"
#include "Spray/Source.h"
#include "Spray/Modifier.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Resource/IResourceManager.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.Emitter", 1, Emitter, ISerializable)

Emitter::Emitter()
:	m_middleAge(0.2f)
,	m_warmUp(0.0f)
{
}

bool Emitter::bind(resource::IResourceManager* resourceManager)
{
	if (!resourceManager->bind(m_shader))
		return false;
	
	if (m_source && !m_source->bind(resourceManager))
		return false;

	return true;
}

Ref< EmitterInstance > Emitter::createInstance()
{
	return new EmitterInstance(this);
}

bool Emitter::serialize(ISerializer& s)
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
