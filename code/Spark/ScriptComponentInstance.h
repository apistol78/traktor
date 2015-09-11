#ifndef traktor_spark_ScriptComponentInstance_H
#define traktor_spark_ScriptComponentInstance_H

#include "Core/Ref.h"
#include "Spark/IComponentInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

	namespace script
	{

class IScriptContext;

	}

	namespace spark
	{

class CharacterInstance;

class T_DLLCLASS ScriptComponentInstance : public IComponentInstance
{
	T_RTTI_CLASS;

public:
	ScriptComponentInstance(CharacterInstance* owner, const IRuntimeClass* scriptClass, ITypedObject* scriptObject);

	virtual void update();

private:
	CharacterInstance* m_owner;
	Ref< const IRuntimeClass > m_scriptClass;
	Ref< ITypedObject > m_scriptObject;
	uint32_t m_methodIdUpdate;
};

	}
}

#endif	// traktor_spark_ScriptComponentInstance_H
