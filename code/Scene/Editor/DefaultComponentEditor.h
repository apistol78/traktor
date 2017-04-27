/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_DefaultComponentEditor_H
#define traktor_scene_DefaultComponentEditor_H

#include "Scene/Editor/IComponentEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityComponentData;

	}

	namespace scene
	{
	
class EntityAdapter;
class SceneEditorContext;

/*! \brief
 * \ingroup Scene
 */	
class T_DLLCLASS DefaultComponentEditor : public IComponentEditor
{
	T_RTTI_CLASS;

public:
	DefaultComponentEditor(SceneEditorContext* context, EntityAdapter* entityAdapter, world::IEntityComponentData* componentData);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const T_OVERRIDE T_FINAL;

private:
	SceneEditorContext* m_context;
	EntityAdapter* m_entityAdapter;
	world::IEntityComponentData* m_componentData;
};

	}
}

#endif	// traktor_scene_DefaultComponentEditor_H
