/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_EntityRendererCache_H
#define traktor_scene_EntityRendererCache_H

#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace world
	{

class Entity;

	}

	namespace scene
	{

class EntityAdapter;
class SceneEditorContext;

/*! \brief Entity adapter lookup from entity accelerator.
 *
 * This class is used by the special entity renderer, EntityRendererAdapter,
 * in order to quickly lookup entity adapter from entity.
 *
 * This is performed by knowing ahead of time which traversal path the
 * renderer is performing thus only accessing children adapters
 * instead of full search.
 */
class EntityRendererCache : public Object
{
	T_RTTI_CLASS;

public:
	EntityRendererCache(SceneEditorContext* context);

	EntityAdapter* begin(const Object* renderable);

	void end();

private:
	SceneEditorContext* m_context;
};

	}
}

#endif	// traktor_scene_EntityRendererCache_H
