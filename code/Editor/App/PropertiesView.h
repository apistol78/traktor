#pragma once

#include <map>
#include "Core/Ref.h"
#include "Ui/Container.h"
#include "Ui/PropertyList/AutoPropertyList.h"

namespace traktor
{

class ISerializable;

	namespace ui
	{

class HierarchicalState;
class PropertyCommandEvent;
class PropertyContentChangeEvent;

	}

	namespace editor
	{

class IEditor;

class PropertiesView
:	public ui::Container
,	public ui::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	PropertiesView(IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() override final;

	void setPropertyObject(ISerializable* object);

	Ref< ISerializable > getPropertyObject();

	bool handleCommand(const ui::Command& command);

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

