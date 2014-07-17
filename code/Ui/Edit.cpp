#include <cwctype>
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/EditValidator.h"
#include "Ui/Itf/IEdit.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Edit", Edit, Widget)

bool Edit::create(Widget* parent, const std::wstring& text, int style, EditValidator* validator)
{
	if (!parent)
		return false;

	if ((m_validator = validator) != 0)
	{
		if (m_validator->validate(text) == EditValidator::VrInvalid)
			return false;
	}

	IEdit* edit = Application::getInstance()->getWidgetFactory()->createEdit(this);
	if (!edit)
	{
		log::error << L"Failed to create native widget peer (Edit)" << Endl;
		return false;
	}

	if (!edit->create(parent->getIWidget(), text, style))
	{
		edit->destroy();
		return false;
	}

	m_widget = edit;

	addEventHandler< KeyEvent >(this, &Edit::eventKey);

	return Widget::create(parent);
}

bool Edit::setValidator(EditValidator* validator)
{
	if (validator)
	{
		if (validator->validate(getText()) == EditValidator::VrInvalid)
			return false;
	}
	m_validator = validator;
	return true;
}

Ref< EditValidator > Edit::getValidator() const
{
	return m_validator;
}

void Edit::setSelection(int from, int to)
{
	T_ASSERT (m_widget);
	static_cast< IEdit* >(m_widget)->setSelection(from, to);
}

void Edit::selectAll()
{
	T_ASSERT (m_widget);
	static_cast< IEdit* >(m_widget)->selectAll();
}

void Edit::eventKey(KeyEvent* event)
{
	wchar_t ch = event->getCharacter();
	if (!m_validator || (!std::iswgraph(ch) && ch != 8))
		return;

	std::wstring text = getText();

	int from, to;
	static_cast< IEdit* >(m_widget)->getSelection(from, to);

	if (ch != 8)
		text = text.substr(0, from) + ch + text.substr(to);
	else
		text = text.substr(0, from - 1) + text.substr(to);

	if (m_validator->validate(text) == EditValidator::VrInvalid)
		event->consume();
}

	}
}
