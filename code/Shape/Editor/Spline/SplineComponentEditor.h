#pragma once

#include "Scene/Editor/DefaultComponentEditor.h"

namespace traktor
{
	namespace shape
	{

/*!
 * \ingroup Shape
 */
class SplineComponentEditor : public scene::DefaultComponentEditor
{
	T_RTTI_CLASS;

public:
	SplineComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;
};

	}
}
