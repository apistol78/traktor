#ifndef traktor_spark_SoundComponent_H
#define traktor_spark_SoundComponent_H

#include "Core/Containers/SmallMap.h"
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
	namespace sound
	{

class Sound;

	}

	namespace spark
	{

/*! \brief Sound component.
 * \ingroup Spark
 */
class T_DLLCLASS SoundComponent : public IComponent
{
	T_RTTI_CLASS;

public:
	virtual Ref< IComponentInstance > createInstance(const Context* context, SpriteInstance* owner) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class CharacterPipeline;

	SmallMap< std::wstring, resource::Id< sound::Sound > > m_sounds;
};

	}
}

#endif	// traktor_spark_SoundComponent_H
