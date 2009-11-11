#include <algorithm>
#include "Ui/MenuItem.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MenuItem", MenuItem, Object)

MenuItem::MenuItem(const Command& command, const std::wstring& text, Bitmap* image)
:	m_command(command)
,	m_text(text)
,	m_image(image)
,	m_enable(true)
{
}

MenuItem::MenuItem(const std::wstring& text, Bitmap* image)
:	m_text(text)
,	m_image(image)
,	m_enable(true)
{
}

void MenuItem::setCommand(const Command& command)
{
	m_command = command;
}

const Command& MenuItem::getCommand() const
{
	return m_command;
}

void MenuItem::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& MenuItem::getText() const
{
	return m_text;
}

void MenuItem::setImage(Bitmap* image)
{
	m_image = image;
}

Ref< Bitmap > MenuItem::getImage() const
{
	return m_image;
}

void MenuItem::setEnable(bool enable)
{
	m_enable = enable;
}

bool MenuItem::isEnable() const
{
	return m_enable;
}

void MenuItem::remove(MenuItem* item)
{
	RefArray< MenuItem >::iterator i = std::find(m_items.begin(), m_items.end(), item);
	m_items.erase(i);
}

void MenuItem::removeAll()
{
	m_items.resize(0);
}

void MenuItem::add(MenuItem* item)
{
	m_items.push_back(item);
}

int MenuItem::count() const
{
	return int(m_items.size());
}

Ref< MenuItem > MenuItem::get(int index)
{
	if (index < 0 || index >= int(m_items.size()))
		return 0;

	return m_items[index];
}

	}
}
