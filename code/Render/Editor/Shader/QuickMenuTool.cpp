/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Render/Editor/Shader/NodeCategories.h"
#include "Render/Editor/Shader/QuickMenuTool.h"
#include "Ui/Edit.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ListBox/ListBox.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class TypeInfoWrapper : public Object
{
public:
	const TypeInfo& m_typeInfo;

	TypeInfoWrapper(const TypeInfo& typeInfo)
	:	m_typeInfo(typeInfo)
	{
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.QuickMenuTool", QuickMenuTool, ui::Dialog)

bool QuickMenuTool::create(ui::Widget* parent)
{
	if (!ui::Dialog::create(parent, L"Quick menu", 100, 200, ui::Dialog::WsCenterParent, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_editFilter = new ui::Edit();
	m_editFilter->create(this, L"");
	m_editFilter->addEventHandler< ui::ContentChangeEvent >(this, &QuickMenuTool::eventFilterChange);
	m_editFilter->addEventHandler< ui::KeyDownEvent >(this, &QuickMenuTool::eventFilterKey);

	m_listBoxSuggestions = new ui::custom::ListBox();
	m_listBoxSuggestions->create(this);
	m_listBoxSuggestions->addEventHandler< ui::SelectionChangeEvent >(this, &QuickMenuTool::eventSuggestionSelect);

	update();
	return true;
}

const TypeInfo* QuickMenuTool::showMenu()
{
	m_editFilter->setText(L"");
	m_editFilter->setFocus();

	updateSuggestions(L"");

	if (showModal() != ui::DrOk)
		return 0;

	Ref< TypeInfoWrapper > typeInfoWrapper = static_cast< TypeInfoWrapper* >(m_listBoxSuggestions->getSelectedData().ptr());
	return typeInfoWrapper ? &typeInfoWrapper->m_typeInfo : 0;
}

void QuickMenuTool::updateSuggestions(const std::wstring& filter)
{
	m_listBoxSuggestions->removeAll();
	for (size_t i = 0; i < sizeof_array(c_nodeCategories); ++i)
	{
		std::wstring typeName = c_nodeCategories[i].type.getName();
		
		size_t p = typeName.find_last_of(L'.');
		if (p != typeName.npos)
			typeName = typeName.substr(p + 1);

		if (startsWith(toLower(typeName), toLower(filter)))
			m_listBoxSuggestions->add(
				typeName.substr(0, p),
				new TypeInfoWrapper(c_nodeCategories[i].type)
			);
	}
	if (!filter.empty())
		m_listBoxSuggestions->select(0);
	else
		m_listBoxSuggestions->select(-1);
}

void QuickMenuTool::eventFilterChange(ui::ContentChangeEvent* event)
{
	// Check isModel in to make sure we haven't called endModal in key down event handler already.
	if (isModal())
	{
		std::wstring filter = m_editFilter->getText();
		updateSuggestions(filter);
	}
}

void QuickMenuTool::eventFilterKey(ui::KeyDownEvent* event)
{
	if (event->getVirtualKey() == ui::VkReturn || event->getVirtualKey() == ui::VkSpace)
	{
		endModal(ui::DrOk);
		event->consume();
	}
	else if (event->getVirtualKey() == ui::VkEscape)
	{
		endModal(ui::DrCancel);
		event->consume();
	}
	else if (event->getVirtualKey() == ui::VkUp)
	{
		int32_t index = m_listBoxSuggestions->getSelected();
		if (index > 0)
			index--;
		else
			index = 0;
		m_listBoxSuggestions->select(index);
		event->consume();
	}
	else if (event->getVirtualKey() == ui::VkDown)
	{
		int32_t index = m_listBoxSuggestions->getSelected();
		if (index < m_listBoxSuggestions->count() - 1)
			index++;
		m_listBoxSuggestions->select(index);
		event->consume();
	}
}

void QuickMenuTool::eventSuggestionSelect(ui::SelectionChangeEvent* event)
{
	if (m_listBoxSuggestions->getSelectedData() != 0)
		endModal(ui::DrOk);
}

	}
}
