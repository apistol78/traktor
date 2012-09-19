#ifndef traktor_scene_IModifier_H
#define traktor_scene_IModifier_H

#include "Core/Object.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace render
	{

class PrimitiveRenderer;

	}

	namespace ui
	{

class Command;

	}

	namespace scene
	{

class TransformChain;

/*! \brief Selection modifier abstraction. */
class IModifier : public Object
{
	T_RTTI_CLASS;

public:
	/*! \name Notifications */
	//\{

	virtual void selectionChanged() = 0;

	virtual bool cursorMoved(const TransformChain& transformChain, const Vector2& cursorPosition, bool mouseDown) = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;

	//\}

	/*! \name Modifications */
	//\{

	virtual void begin(const TransformChain& transformChain) = 0;

	virtual void apply(const TransformChain& transformChain, const Vector4& screenDelta, const Vector4& viewDelta) = 0;

	virtual void end(const TransformChain& transformChain) = 0;

	//\}

	/*! \name Preview */
	//\{

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) const = 0;

	//\}
};

	}
}

#endif	// traktor_scene_IModifier_H
