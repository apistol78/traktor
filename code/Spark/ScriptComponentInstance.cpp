#include "Script/IScriptContext.h"
#include "Spark/CharacterInstance.h"
#include "Spark/ScriptComponentInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ScriptComponentInstance", ScriptComponentInstance, IComponentInstance)

ScriptComponentInstance::ScriptComponentInstance(CharacterInstance* owner)
:	m_owner(owner)
{
}

void ScriptComponentInstance::update()
{
	//m_script->executeFunction("update", 0, 0);
}

	}
}
