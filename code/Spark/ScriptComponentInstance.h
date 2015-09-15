#ifndef traktor_spark_ScriptComponentInstance_H
#define traktor_spark_ScriptComponentInstance_H

#include "Core/Ref.h"
#include "Resource/Proxy.h"
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
	namespace script
	{

class IScriptContext;

	}

	namespace spark
	{

class CharacterInstance;

/*! \brief Script component instance.
 * \ingroup Spark
 */
class T_DLLCLASS ScriptComponentInstance : public IComponentInstance
{
	T_RTTI_CLASS;

public:
	ScriptComponentInstance(CharacterInstance* owner);

	virtual void update();

private:
	friend class ScriptComponent;

	CharacterInstance* m_owner;
	resource::Proxy< script::IScriptContext > m_script;
};

	}
}

#endif	// traktor_spark_ScriptComponentInstance_H
