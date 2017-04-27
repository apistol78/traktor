/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_DefaultComponentEditorFactory_H
#define traktor_scene_DefaultComponentEditorFactory_H

#include "Scene/Editor/IComponentEditorFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

/*! \brief
 * \ingroup Scene
 */
class T_DLLCLASS DefaultComponentEditorFactory : public IComponentEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getComponentDataTypes() const T_OVERRIDE T_FINAL;

	virtual Ref< IComponentEditor > createComponentEditor(SceneEditorContext* context, EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_scene_DefaultComponentEditorFactory_H
