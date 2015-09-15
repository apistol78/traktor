#ifndef traktor_spark_DisplayList_H
#define traktor_spark_DisplayList_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"

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

class CharacterInstance;

/*! \brief Display list.
 * \ingroup Spark
 */
class T_DLLCLASS DisplayList : public Object
{
	T_RTTI_CLASS;

public:
	struct Layer
	{
		Ref< CharacterInstance > instance;
	};

	void place(int32_t depth, CharacterInstance* instance);

	void remove(int32_t depth);

	const SmallMap< int32_t, Layer >& getLayers() const;

	void getCharacters(RefArray< CharacterInstance >& outCharacters) const;

private:
	SmallMap< int32_t, Layer > m_layers;
};

	}
}

#endif	// traktor_spark_DisplayList_H
