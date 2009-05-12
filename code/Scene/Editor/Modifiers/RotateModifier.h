#ifndef traktor_scene_RotateModifier_H
#define traktor_scene_RotateModifier_H

#include "Scene/Editor/Modifier.h"

namespace traktor
{
	namespace scene
	{

/*! \brief Rotation modifier. */
class RotateModifier : public Modifier
{
	T_RTTI_CLASS(RotateModifier)

public:
	virtual void draw(
		SceneEditorContext* context,
		const Matrix44& worldTransform,
		render::PrimitiveRenderer* primitiveRenderer,
		bool active,
		int button
	);

	virtual void adjust(
		SceneEditorContext* context,
		const Matrix44& viewTransform,
		const Vector2& screenDelta,
		int button,
		Matrix44& outTransform
	);
};

	}
}

#endif	// traktor_scene_RotateModifier_H
