#include "Spark/CharacterInstance.h"
#include "Spark/DisplayList.h"
#include "Spark/DisplayRenderer.h"
#include "Spark/StageInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.StageInstance", StageInstance, Object)

StageInstance::StageInstance(const Stage* stage)
:	m_stage(stage)
{
}

void StageInstance::update()
{
	for (RefArray< CharacterInstance >::const_iterator i = m_characters.begin(); i != m_characters.end(); ++i)
		(*i)->update();
}

void StageInstance::build(DisplayRenderer* displayRenderer, uint32_t frame) const
{
	// \hack
	DisplayList displayList;
	for (uint32_t i = 0; i < m_characters.size(); ++i)
		displayList.place(i, m_characters[i]);

	displayRenderer->build(&displayList, frame);
}

const resource::Proxy< script::IScriptContext >& StageInstance::getScriptContext() const
{
	return m_script;
}

	}
}
