/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_animation_ClothEntityEditor_H
#define traktor_animation_ClothEntityEditor_H

#include "Scene/Editor/DefaultEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class T_DLLEXPORT ClothEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	ClothEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_animation_ClothEntityEditor_H
