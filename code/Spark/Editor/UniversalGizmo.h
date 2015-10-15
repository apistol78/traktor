#ifndef traktor_spark_UniversalGizmo_H
#define traktor_spark_UniversalGizmo_H

#include "Spark/Editor/IGizmo.h"

namespace traktor
{
	namespace spark
	{

class EditContext;

class UniversalGizmo : public IGizmo
{
	T_RTTI_CLASS;

public:
	UniversalGizmo(EditContext* editContext);

	virtual void attach(CharacterAdapter* adapter) T_OVERRIDE T_FINAL;

	virtual void detach(CharacterAdapter* adapter) T_OVERRIDE T_FINAL;

	virtual void mouseDown(ui::Widget* widget, CharacterAdapter* adapter, const Vector2& position) T_OVERRIDE T_FINAL;

	virtual void mouseUp(ui::Widget* widget, CharacterAdapter* adapter, const Vector2& position) T_OVERRIDE T_FINAL;

	virtual void mouseMove(ui::Widget* widget, CharacterAdapter* adapter, const Vector2& position) T_OVERRIDE T_FINAL;

	virtual void paint(CharacterAdapter* adapter, render::PrimitiveRenderer* primitiveRenderer) T_OVERRIDE T_FINAL;

private:
	enum EditMode
	{
		EmNone,
		EmTranslate,
		EmScale,
		EmRotate
	};

	EditContext* m_editContext;
	Vector2 m_center;
	Vector2 m_scaleCorners[4];
	Vector2 m_scaleEdges[4];
	Vector2 m_rotateCorners[4];
	EditMode m_mode;
	Vector2 m_lastPosition;
};

	}
}

#endif	// traktor_spark_UniversalGizmo_H
