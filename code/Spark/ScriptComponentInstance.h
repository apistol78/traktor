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

class IRuntimeClass;

	namespace spark
	{

class SpriteInstance;

/*! \brief Script component instance.
 * \ingroup Spark
 */
class T_DLLCLASS ScriptComponentInstance : public IComponentInstance
{
	T_RTTI_CLASS;

public:
	ScriptComponentInstance(SpriteInstance* owner, const resource::Proxy< IRuntimeClass >& clazz);

	virtual void update();

private:
	SpriteInstance* m_owner;
	resource::Proxy< IRuntimeClass > m_class;
	Ref< ITypedObject > m_object;
	uint32_t m_methodUpdate;
};

	}
}

#endif	// traktor_spark_ScriptComponentInstance_H
