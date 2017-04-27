/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

class Character;

/*! \brief Display list.
 * \ingroup Spark
 */
class T_DLLCLASS DisplayList : public Object
{
	T_RTTI_CLASS;

public:
	struct Layer
	{
		Ref< Character > instance;
	};

	void place(int32_t depth, Character* instance);

	void remove(int32_t depth);

	void remove(Character* instance);

	const SmallMap< int32_t, Layer >& getLayers() const;

	void getCharacters(RefArray< Character >& outCharacters) const;

private:
	SmallMap< int32_t, Layer > m_layers;
};

	}
}

#endif	// traktor_spark_DisplayList_H
