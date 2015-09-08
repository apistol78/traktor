#ifndef traktor_spark_ICharacterResource_H
#define traktor_spark_ICharacterResource_H

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
	namespace db
	{

class Instance;

	}

	namespace render
	{

class IRenderSystem;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace spark
	{

class Character;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS ICharacterResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< Character > create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, db::Instance* resourceInstance) const = 0;
};

	}
}

#endif	// traktor_spark_ICharacterResource_H
