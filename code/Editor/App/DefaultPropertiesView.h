#pragma once

#include <map>
#include "Core/Ref.h"
#include "Editor/PropertiesView.h"
#include "Ui/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace ui
	{

class HierarchicalState;
class PropertyCommandEvent;
class PropertyContentChangeEvent;

	}

	namespace editor
	{

class IEditor;

class DefaultPropertiesView
:	public PropertiesView
,	public ui::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	explicit DefaultPropertiesView(IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void setPropertyObject(ISerializable* object) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	// ui::PropertyList::IPropertyGuidResolver

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const override final;

private:
	IEditor* m_editor;
	Ref< ui::AutoPropertyList > m_propertyList;
	Ref< ISerializable > m_propertyObject;
	std::map< const TypeInfo*, Ref< ui::HierarchicalState > > m_states;

	void eventPropertyCommand(ui::PropertyCommandEvent* event);

	void eventPropertyChange(ui::PropertyContentChangeEvent* event);
};

	}
}

