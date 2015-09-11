#ifndef traktor_spark_StageInstance_H
#define traktor_spark_StageInstance_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IScriptContext;

	}

	namespace spark
	{

class CharacterInstance;
class DisplayRenderer;
class Stage;

class T_DLLCLASS StageInstance : public Object
{
	T_RTTI_CLASS;

public:
	StageInstance(const Stage* stage);

	void update();

	void build(DisplayRenderer* displayRenderer, uint32_t frame) const;

	const resource::Proxy< script::IScriptContext >& getScriptContext() const;

private:
	friend class Stage;

	Ref< const Stage > m_stage;
	resource::Proxy< script::IScriptContext > m_script;
	RefArray< CharacterInstance > m_characters;
};

	}
}

#endif	// traktor_spark_StageInstance_H
