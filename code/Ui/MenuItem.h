#ifndef traktor_ui_MenuItem_H
#define traktor_ui_MenuItem_H

#include <string>
#include <vector>
#include "Core/RefArray.h"
#include "Ui/Associative.h"
#include "Ui/Command.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;

/*! \brief Menu item.
 * \ingroup UI
 */
class T_DLLCLASS MenuItem
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	MenuItem(const Command& command, const std::wstring& text, Bitmap* image = 0);

	MenuItem(const std::wstring& text, Bitmap* image = 0);

	void setCommand(const Command& command);

	const Command& getCommand() const;

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setImage(Bitmap* image);

	Ref< Bitmap > getImage() const;

	void setEnable(bool enable);

	bool isEnable() const;

	void add(MenuItem* item);

	void remove(MenuItem* item);

	void removeAll();

	int count() const;

	Ref< MenuItem > get(int index);

private:
	Command m_command;
	std::wstring m_text;
	Ref< Bitmap > m_image;
	bool m_enable;
	RefArray< MenuItem > m_items;
};

	}
}

#endif	// traktor_ui_MenuItem_H
