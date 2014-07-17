#ifndef traktor_world_PostProcessStepProperties_H
#define traktor_world_PostProcessStepProperties_H

#include "Core/Object.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{
		namespace custom
		{

class PropertyCommandEvent;
class PropertyContentChangeEvent;

		}
	}

	namespace world
	{

class PostProcessStepProperties
:	public ui::EventSubject
,	public ui::custom::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	PostProcessStepProperties(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	void destroy();

	void set(PostProcessStep* step);

private:
	editor::IEditor* m_editor;
	Ref< ui::custom::AutoPropertyList > m_propertyList;

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;

	void eventPropertyCommand(ui::custom::PropertyCommandEvent* event);

	void eventPropertyChange(ui::custom::PropertyContentChangeEvent* event);
};

	}
}

#endif	// traktor_world_PostProcessStepProperties_H
