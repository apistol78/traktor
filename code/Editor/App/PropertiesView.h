#ifndef traktor_editor_PropertiesView_H
#define traktor_editor_PropertiesView_H

#include <map>
#include "Core/Ref.h"
#include "Ui/Container.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{

class ISerializable;

	namespace ui
	{

class HierarchicalState;

		namespace custom
		{

class GradientStatic;

		}
	}

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

	void setPropertyObject(ISerializable* object);

	Ref< ISerializable > getPropertyObject();

	bool handleCommand(const ui::Command& command);

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;

private:
	IEditor* m_editor;
	Ref< ui::custom::AutoPropertyList > m_propertyList;
	Ref< ui::custom::GradientStatic > m_staticHelp;
	Ref< ISerializable > m_propertyObject;
	std::map< const TypeInfo*, Ref< ui::HierarchicalState > > m_states;

	void updateHelp();

	void eventPropertyCommand(ui::Event* event);

	void eventPropertyChange(ui::Event* event);

	void eventPropertySelect(ui::Event* event);
};

	}
}

#endif	// traktor_editor_PropertiesView_H
