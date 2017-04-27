/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Math/Color4ub.h"
#include "Ui/Application.h"
#include "Ui/RichEdit.h"
#include "Ui/Itf/IRichEdit.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.RichEdit", RichEdit, Widget)

bool RichEdit::create(Widget* parent, const std::wstring& text, int style)
{
	if (!parent)
		return false;

	IRichEdit* richEdit = Application::getInstance()->getWidgetFactory()->createRichEdit(this);
	if (!richEdit)
	{
		log::error << L"Failed to create native widget peer (RichEdit)" << Endl;
		return false;
	}

	if (!richEdit->create(parent->getIWidget(), text, style))
	{
		richEdit->destroy();
		return false;
	}

	m_widget = richEdit;
	
	return Widget::create(parent, style);
}

int RichEdit::addAttribute(const Color4ub& textColor, const Color4ub& backColor, bool bold, bool italic, bool underline)
{
	return static_cast< IRichEdit* >(m_widget)->addAttribute(textColor, backColor, bold, italic, underline);
}

void RichEdit::setAttribute(int start, int length, int attribute)
{
	static_cast< IRichEdit* >(m_widget)->setAttribute(start, length, attribute);
}

void RichEdit::clear(bool attributes, bool content)
{
	static_cast< IRichEdit* >(m_widget)->clear(attributes, content);
}

void RichEdit::insert(const std::wstring& text)
{
	static_cast< IRichEdit* >(m_widget)->insert(text);
}

int RichEdit::getCaretOffset() const
{
	return static_cast< IRichEdit* >(m_widget)->getCaretOffset();
}

int RichEdit::getLineFromOffset(int offset) const
{
	return static_cast< IRichEdit* >(m_widget)->getLineFromOffset(offset);
}

int RichEdit::getLineCount() const
{
	return static_cast< IRichEdit* >(m_widget)->getLineCount();
}

int RichEdit::getLineOffset(int line) const
{
	return static_cast< IRichEdit* >(m_widget)->getLineOffset(line);
}

int RichEdit::getLineLength(int line) const
{
	return static_cast< IRichEdit* >(m_widget)->getLineLength(line);
}

std::wstring RichEdit::getLine(int line) const
{
	return static_cast< IRichEdit* >(m_widget)->getLine(line);
}

bool RichEdit::redo()
{
	return static_cast< IRichEdit* >(m_widget)->redo();
}

bool RichEdit::undo()
{
	return static_cast< IRichEdit* >(m_widget)->undo();
}

	}
}

