#pragma once

#include "Core/Ref.h"
#include "Ui/Dialog.h"

namespace traktor
{
	namespace ui
	{

class LogList;

	}

	namespace amalgam
	{

class ErrorDialog : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	bool create();

	void addErrorString(const std::wstring& errorString);

private:
	Ref< ui::LogList > m_listLog;

	void eventButtonCopyQuit(ui::ButtonClickEvent* event);

	void eventButtonClickQuit(ui::ButtonClickEvent* event);
};

	}
}

