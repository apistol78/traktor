#include "Ui/Unite/EditUnite.h"

namespace traktor
{
	namespace ui
	{

EditUnite::EditUnite(IWidgetFactory* nativeWidgetFactory, EventSubject* owner)
:	WidgetUniteImpl< IEdit >(nativeWidgetFactory, owner)
{
}

bool EditUnite::create(IWidget* parent, const std::wstring& text, int style)
{
	m_nativeWidget = m_nativeWidgetFactory->createUserWidget(this);
	return true;
}

void EditUnite::setSelection(int from, int to)
{
}

void EditUnite::getSelection(int& outFrom, int& outTo) const
{
}

void EditUnite::selectAll()
{
}

Size EditUnite::getPreferedSize() const
{
	return Size(256, 16);
}

	}
}
