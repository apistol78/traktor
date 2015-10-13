#ifndef traktor_spark_IGizmo_H
#define traktor_spark_IGizmo_H

#include "Core/Object.h"
#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace render
	{

class PrimitiveRenderer;

	}

	namespace ui
	{

class Widget;

	}

	namespace spark
	{

class CharacterAdapter;

class IGizmo : public Object
{
	T_RTTI_CLASS;

public:
	virtual void attach(CharacterAdapter* adapter) = 0;

	virtual void detach(CharacterAdapter* adapter) = 0;

	virtual void mouseDown(ui::Widget* widget, CharacterAdapter* adapter, const Vector2& position) = 0;

	virtual void mouseUp(ui::Widget* widget, CharacterAdapter* adapter, const Vector2& position) = 0;

	virtual void mouseMove(ui::Widget* widget, CharacterAdapter* adapter, const Vector2& position) = 0;

	virtual void paint(CharacterAdapter* adapter, render::PrimitiveRenderer* primitiveRenderer) = 0;
};

	}
}

#endif	// traktor_spark_IGizmo_H
