/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_IComponentEditor_H
#define traktor_scene_IComponentEditor_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class PrimitiveRenderer;

	}

	namespace scene
	{
	
/*! \brief
 * \ingroup Scene
 */	
class T_DLLCLASS IComponentEditor : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Draw guide for component.
	 *
	 * A guide is the wire overlay
	 * in the 3d editor view.
	 *
	 * \param primitiveRenderer Primitive wire renderer.
	 */
	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const = 0;
};

	}
}

#endif	// traktor_scene_IComponentEditor_H
