#ifndef traktor_editor_DefaultObjectEditor_H
#define traktor_editor_DefaultObjectEditor_H

#include "Editor/IObjectEditor.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

class DefaultObjectEditor
:	public IObjectEditor
,	public ui::custom::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	DefaultObjectEditor(IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy();

	virtual void apply();

private:
	IEditor* m_editor;
	Ref< ui::custom::AutoPropertyList > m_propertyList;

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;

	void eventPropertyCommand(ui::Event* event);
};

	}
}

#endif	// traktor_editor_DefaultObjectEditor_H
