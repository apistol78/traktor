#pragma once

#include <Ui/ConfigDialog.h>
#include <Ui/ListBox/ListBox.h>

namespace traktor
{
	namespace sb
	{

class Solution;

class ConfigurationsDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	enum ActionType
	{
		AtNew,
		AtRename,
		AtRemove
	};

	struct Action
	{
		ActionType type;
		std::wstring name;
		std::wstring current;
	};

	bool create(ui::Widget* parent, Solution* solution);

	const std::vector< Action >& getActions() const;

private:
	Ref< ui::ListBox > m_listConfigurations;
	std::vector< Action > m_actions;

	void eventButtonNew(ui::ButtonClickEvent* event);

	void eventButtonRename(ui::ButtonClickEvent* event);

	void eventButtonRemove(ui::ButtonClickEvent* event);
};

	}
}

