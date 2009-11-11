#ifndef traktor_editor_PropertiesView_H
#define traktor_editor_PropertiesView_H

#include "Core/Heap/Ref.h"
#include "Ui/Container.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{

class Serializable;

	namespace editor
	{

class IEditor;

class PropertiesView
:	public ui::Container
,	public ui::custom::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS(PropertiesView)

public:
	PropertiesView(IEditor* editor);

	bool create(ui::Widget* parent);

	void destroy();

	void setPropertyObject(Serializable* object, Serializable* outer);

	Ref< Serializable > getPropertyObject();

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;

private:
	IEditor* m_editor;
	Ref< ui::custom::AutoPropertyList > m_propertyList;
	Ref< Serializable > m_propertyObject;

	void eventPropertyCommand(ui::Event* event);

	void eventPropertyChange(ui::Event* event);
};

	}
}

#endif	// traktor_editor_PropertiesView_H
