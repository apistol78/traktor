#ifndef traktor_editor_PropertiesView_H
#define traktor_editor_PropertiesView_H

#include "Ui/Container.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{

class ISerializable;

	namespace editor
	{

class IEditor;

class PropertiesView
:	public ui::Container
,	public ui::custom::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	PropertiesView(IEditor* editor);

	bool create(ui::Widget* parent);

	void destroy();

	void setPropertyObject(ISerializable* object, ISerializable* outer);

	Ref< ISerializable > getPropertyObject();

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;

private:
	IEditor* m_editor;
	Ref< ui::custom::AutoPropertyList > m_propertyList;
	Ref< ISerializable > m_propertyObject;

	void eventPropertyCommand(ui::Event* event);

	void eventPropertyChange(ui::Event* event);
};

	}
}

#endif	// traktor_editor_PropertiesView_H
