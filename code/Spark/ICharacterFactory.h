#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
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

class Matrix33;

	namespace spark
	{

class ActionContext;
class ActionObject;
class Character;
class CharacterInstance;
class Dictionary;
class IActionVMImage;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS ICharacterFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< CharacterInstance > createInstance(
		const Character* character,
		int32_t depth,
		ActionContext* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const = 0;

	virtual void removeInstance(CharacterInstance* instance, int32_t depth) const = 0;
};

	}
}
