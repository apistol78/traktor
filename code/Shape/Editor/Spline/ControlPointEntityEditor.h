#pragma once

#include "Scene/Editor/DefaultEntityEditor.h"

namespace traktor
{
	namespace shape
	{

/*! \brief
 * \ingroup Shape
 */
class ControlPointEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	ControlPointEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;
};

	}
}
