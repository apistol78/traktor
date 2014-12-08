#ifndef ConfigurationsDialog_H
#define ConfigurationsDialog_H

#include <Ui/ConfigDialog.h>
#include <Ui/ListBox.h>

class Solution;

class ConfigurationsDialog : public traktor::ui::ConfigDialog
{
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

	bool create(traktor::ui::Widget* parent, Solution* solution);

	const std::vector< Action >& getActions() const;

private:
	traktor::Ref< traktor::ui::ListBox > m_listConfigurations;
	std::vector< Action > m_actions;

	void eventButtonNew(traktor::ui::ButtonClickEvent* event);

	void eventButtonRename(traktor::ui::ButtonClickEvent* event);

	void eventButtonRemove(traktor::ui::ButtonClickEvent* event);
};

#endif	// ConfigurationsDialog_H
