#ifndef traktor_scene_IModifier_H
#define traktor_scene_IModifier_H

#include "Core/Object.h"
#include "Core/Math/Matrix44.h"

namespace traktor
{
	namespace render
	{

class PrimitiveRenderer;

	}

	namespace scene
	{

class SceneEditorContext;

/*! \brief Selection modifier abstraction. */
class IModifier : public Object
{
	T_RTTI_CLASS(IModifier)

public:
	virtual void draw(
		SceneEditorContext* context,
		const Matrix44& viewTransform,
		const Matrix44& worldTransform,
		render::PrimitiveRenderer* primitiveRenderer,
		int button
	) = 0;

	virtual void adjust(
		SceneEditorContext* context,
		const Matrix44& viewTransform,
		const Vector4& screenDelta,
		const Vector4& viewDelta,
		const Vector4& worldDelta,
		int button,
		Matrix44& outTransform
	) = 0;
};

	}
}

#endif	// traktor_scene_IModifier_H
