#ifndef traktor_editor_SearchToolDialog_H
#define traktor_editor_SearchToolDialog_H

#include "Ui/Dialog.h"

namespace traktor
{

class Job;

	namespace ui
	{

class Button;
class CheckBox;
class Edit;

		namespace custom
		{

class GridView;
class ProgressBar;

		}
	}

	namespace editor
	{

class IEditor;

class SearchToolDialog : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	SearchToolDialog(IEditor* editor);

	virtual void destroy();

	bool create(ui::Widget* parent);

private:
	IEditor* m_editor;
	Ref< ui::Edit > m_editSearch;
	Ref< ui::CheckBox > m_checkCaseSensitive;
	Ref< ui::Button > m_buttonFind;
	Ref< ui::Button > m_buttonSaveAs;
	Ref< ui::custom::ProgressBar > m_progressBar;
	Ref< ui::custom::GridView > m_gridResults;
	Ref< Job > m_jobSearch;

	void eventButtonSearchClick(ui::ButtonClickEvent* event);

	void eventButtonSaveAsClick(ui::ButtonClickEvent* event);

	void eventGridResultDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventGridResultButtonUp(ui::MouseButtonUpEvent* event);

	void eventTimer(ui::TimerEvent* event);

	void eventClose(ui::CloseEvent* event);

	void jobSearch(std::wstring needle, bool caseSensitive);
};

	}
}

#endif	// traktor_editor_SearchToolDialog_H
