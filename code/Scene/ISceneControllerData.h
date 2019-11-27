#pragma once

#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class EntityData;

	}

	namespace scene
	{

class ISceneController;

/*! Scene controller data.
 * \ingroup Scene
 *
 * Scene controller data is a persistent
 * means of storing specific scene controller
 * data on disc.
 */
class T_DLLCLASS ISceneControllerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Create controller instance.
	 *
	 * \param entityProducts Entites created from entity data.
	 * \param editor True if scene is created in editor.
	 * \return Controller instance.
	 */
	virtual Ref< ISceneController > createController(const SmallMap< const world::EntityData*, Ref< world::Entity > >& entityProducts, bool editor) const = 0;
};

	}
}

