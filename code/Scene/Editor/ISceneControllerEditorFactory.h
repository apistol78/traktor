/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_ISceneControllerEditorFactory_H
#define traktor_scene_ISceneControllerEditorFactory_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class ISceneControllerEditor;

/*! \brief
 * \ingroup Scene
 */
class T_DLLCLASS ISceneControllerEditorFactory : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Get supported set of controller data types.
	 *
	 * \return Set of controller data types.
	 */
	virtual const TypeInfoSet getControllerDataTypes() const = 0;

	/*! \brief Create controller editor instance.
	 *
	 * \param controllerDataType Controller data type.
	 */
	virtual Ref< ISceneControllerEditor > createControllerEditor(const TypeInfo& controllerDataType) const = 0;
};

	}
}

#endif	// traktor_scene_ISceneControllerEditorFactory_H
