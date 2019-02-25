#pragma once

#include "Scene/Editor/DefaultEntityEditor.h"

namespace traktor
{
	namespace shape
	{

/*! \brief
 * \ingroup Shape
 */
class SplineEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	SplineEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual bool isGroup() const override final;

	virtual bool addChildEntity(scene::EntityAdapter* childEntityAdapter) const override final;

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;
};

	}
}
