#ifndef traktor_scene_RotateModifier_H
#define traktor_scene_RotateModifier_H

#include "Scene/Editor/IModifier.h"

namespace traktor
{
	namespace scene
	{

/*! \brief Rotation modifier. */
class RotateModifier : public IModifier
{
	T_RTTI_CLASS;

public:
	virtual void draw(
		SceneEditorContext* context,
		const Matrix44& viewTransform,
		const Transform& worldTransform,
		render::PrimitiveRenderer* primitiveRenderer,
		int button
	);

	virtual void adjust(
		SceneEditorContext* context,
		const Matrix44& viewTransform,
		const Vector4& screenDelta,
		const Vector4& viewDelta,
		const Vector4& worldDelta,
		int button,
		Transform& outTransform
	);
};

	}
}

#endif	// traktor_scene_RotateModifier_H
