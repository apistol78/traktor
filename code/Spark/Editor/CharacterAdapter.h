#ifndef traktor_spark_CharacterAdapter_H
#define traktor_spark_CharacterAdapter_H

#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{

class Matrix33;

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

class CharacterData;
class CharacterInstance;
class IGizmo;

class CharacterAdapter : public Object
{
	T_RTTI_CLASS;

public:
	CharacterAdapter();

	const std::wstring& getName() const;

	void setTransform(const Matrix33& transform);

	const Matrix33& getTransform() const;

	CharacterData* getCharacterData();

	CharacterInstance* getCharacterInstance();

	CharacterAdapter* getParent();

	const RefArray< CharacterAdapter >& getChildren();

	void unlink();

	void attachGizmo(IGizmo* gizmo);

	void detachGizmo();

	void mouseDown(ui::Widget* widget, const Vector2& position);
	
	void mouseUp(ui::Widget* widget, const Vector2& position);

	void mouseMove(ui::Widget* widget, const Vector2& position);

	void paint(render::PrimitiveRenderer* primitiveRenderer);

	void select();

	void deselect();

	bool isSelected() const;

	bool isChildOfExternal() const;

private:
	friend class CharacterAdapterBuilder;

	std::wstring m_name;
	Ref< CharacterData > m_characterData;
	Ref< CharacterInstance > m_characterInstance;
	CharacterAdapter* m_parent;
	RefArray< CharacterAdapter > m_children;
	Ref< IGizmo > m_gizmo;
	bool m_selected;
};

	}
}

#endif	// traktor_spark_CharacterAdapter_H
