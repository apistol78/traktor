#pragma once

#include "Scene/Editor/DefaultComponentEditor.h"

namespace traktor
{
	namespace shape
	{

/*!
 * \ingroup Shape
 */
class ControlPointComponentEditor : public scene::DefaultComponentEditor
{
	T_RTTI_CLASS;

public:
	ControlPointComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;
};

	}
}
