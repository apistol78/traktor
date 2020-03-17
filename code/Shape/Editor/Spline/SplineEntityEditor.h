#pragma once

#include "Scene/Editor/DefaultEntityEditor.h"

namespace traktor
{
	namespace shape
	{

/*!
 * \ingroup Shape
 */
class SplineEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	SplineEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;
};

	}
}
