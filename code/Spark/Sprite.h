#ifndef traktor_spark_Sprite_H
#define traktor_spark_Sprite_H

#include "Core/RefArray.h"
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

class Shape;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS Sprite : public Character
{
	T_RTTI_CLASS;

public:
	virtual Ref< CharacterInstance > createInstance(const CharacterInstance* parent, resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

private:
	resource::Id< Shape > m_shape;
	RefArray< Character > m_children;
};

	}
}

#endif	// traktor_spark_Sprite_H
