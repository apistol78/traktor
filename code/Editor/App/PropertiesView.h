#ifndef traktor_editor_PropertiesView_H
#define traktor_editor_PropertiesView_H

#include "Core/Heap/Ref.h"
#include "Ui/Container.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace editor
	{

class Editor;

class PropertiesView
:	public ui::Container
,	public ui::custom::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS(PropertiesView)

public:
	PropertiesView(Editor* editor);

	bool create(ui::Widget* parent);

	void destroy();

	void setPropertyObject(Object* propertyObject);

	Object* getPropertyObject();

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;

private:
	Editor* m_editor;
	Ref< ui::custom::AutoPropertyList > m_propertyList;
	Ref< Object > m_propertyObject;

	void eventPropertyCommand(ui::Event* event);

	void eventPropertyChange(ui::Event* event);
};

	}
}

#endif	// traktor_editor_PropertiesView_H
