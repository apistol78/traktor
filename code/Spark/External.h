#ifndef traktor_spark_External_H
#define traktor_spark_External_H

#include "Resource/Id.h"
#include "Spark/Character.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! \brief External reference to character.
 * \ingroup Spark
 */
class T_DLLCLASS External : public Character
{
	T_RTTI_CLASS;

public:
	virtual Ref< CharacterInstance > createInstance(const CharacterInstance* parent, resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	friend class CharacterPipeline;

	resource::Id< Character > m_reference;
};

	}
}

#endif	// traktor_spark_External_H
