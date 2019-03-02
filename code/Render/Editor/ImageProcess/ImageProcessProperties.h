#pragma once

#include "Core/Object.h"
#include "Ui/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class PropertyCommandEvent;
class PropertyContentChangeEvent;

	}

	namespace render
	{

class ImageProcessProperties
:	public ui::EventSubject
,	public ui::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	ImageProcessProperties(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	void destroy();

	void set(ISerializable* object);

private:
	editor::IEditor* m_editor;
	Ref< ui::AutoPropertyList > m_propertyList;

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const override final;

	void eventPropertyCommand(ui::PropertyCommandEvent* event);

	void eventPropertyChange(ui::PropertyContentChangeEvent* event);
};

	}
}

