/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_IComponentEditorFactory_H
#define traktor_scene_IComponentEditorFactory_H

#include "Core/Object.h"
#include "Core/Ref.h"

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
class IComponentEditor;
class SceneEditorContext;

/*! \brief
 * \ingroup Scene
 */
class T_DLLCLASS IComponentEditorFactory : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Get supported set of components.
	 *
	 * \return Type Set of IEntityComponentData types.
	 */
	virtual const TypeInfoSet getComponentDataTypes() const = 0;

	/*! \brief Create component editor.
	 *
	 * \param context Scene editor context.
	 * \param entityAdapter Owner entity adapter.
	 * \param componentData Component data.
	 * \return Component editor instance.
	 */
	virtual Ref< IComponentEditor > createComponentEditor(SceneEditorContext* context, EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const = 0;
};

	}
}

#endif	// traktor_scene_IComponentEditorFactory_H
