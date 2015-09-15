#ifndef traktor_spark_ScriptComponent_H
#define traktor_spark_ScriptComponent_H

#include "Resource/Id.h"
#include "Spark/IComponent.h"

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

/*! \brief Script component.
 * \ingroup Spark
 */
class T_DLLCLASS ScriptComponent : public IComponent
{
	T_RTTI_CLASS;

public:
	virtual Ref< IComponentInstance > createInstance(CharacterInstance* owner, resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	friend class CharacterPipeline;

	resource::Id< IRuntimeClass > m_class;
};

	}
}

#endif	// traktor_spark_ScriptComponent_H
