#ifndef traktor_spark_IComponent_H
#define traktor_spark_IComponent_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class ISoundPlayer;

	}

	namespace spark
	{

class SpriteInstance;
class IComponentInstance;

/*! \brief Character component interface.
 * \ingroup Spark
 */
class T_DLLCLASS IComponent : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IComponentInstance > createInstance(SpriteInstance* owner, resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer) const = 0;
};

	}
}

#endif	// traktor_spark_IComponent_H
