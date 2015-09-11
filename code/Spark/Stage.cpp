#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Script/IScriptContext.h"
#include "Spark/Character.h"
#include "Spark/Stage.h"
#include "Spark/StageInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.Stage", 0, Stage, ISerializable)

Ref< StageInstance > Stage::createInstance(resource::IResourceManager* resourceManager) const
{
	Ref< StageInstance > stageInstance = new StageInstance(this);

	if (!resourceManager->bind(m_script, stageInstance->m_script))
		return 0;

	for (RefArray< Character >::const_iterator i = m_characters.begin(); i != m_characters.end(); ++i)
	{
		Ref< CharacterInstance > character = (*i)->createInstance(stageInstance, 0, resourceManager);
		if (!character)
			return 0;

		stageInstance->m_characters.push_back(character);
	}

	return stageInstance;
}

void Stage::serialize(ISerializer& s)
{
	s >> resource::Member< script::IScriptContext >(L"script", m_script);
	s >> MemberRefArray< Character >(L"characters", m_characters);
}

	}
}
