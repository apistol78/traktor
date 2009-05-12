#ifndef traktor_editor_DefaultObjectEditor_H
#define traktor_editor_DefaultObjectEditor_H

#include "Core/Heap/Ref.h"
#include "Editor/ObjectEditor.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace editor
	{

class Editor;

class DefaultObjectEditor
:	public ObjectEditor
,	public ui::custom::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS(ObjectEditor)

public:
	DefaultObjectEditor(Editor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, Object* object);

	virtual void destroy();

	virtual void apply();

private:
	Editor* m_editor;
	Ref< ui::custom::AutoPropertyList > m_propertyList;

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;

	void eventPropertyCommand(ui::Event* event);
};

	}
}

#endif	// traktor_editor_DefaultObjectEditor_H
