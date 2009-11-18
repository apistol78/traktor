#ifndef traktor_scene_ISceneControllerData_H
#define traktor_scene_ISceneControllerData_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityBuilder;
class IEntityManager;

	}

	namespace scene
	{

class ISceneController;

class T_DLLCLASS ISceneControllerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< ISceneController > createController(world::IEntityBuilder* entityBuilder, world::IEntityManager* entityManager) const = 0;
};

	}
}

#endif	// traktor_scene_ISceneControllerData_H
