/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	MenuItem(const Command& command, const std::wstring& text, bool checkBox, Bitmap* image);

	MenuItem(const std::wstring& text, bool checkBox, Bitmap* image);

	MenuItem(const Command& command, const std::wstring& text, Bitmap* image);

	MenuItem(const std::wstring& text, Bitmap* image);

	MenuItem(const Command& command, const std::wstring& text);

	MenuItem(const std::wstring& text);

	void setCommand(const Command& command);

	const Command& getCommand() const;

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setCheckBox(bool checkBox);

	bool getCheckBox() const;

	void setImage(Bitmap* image);

	Ref< Bitmap > getImage() const;

	void setEnable(bool enable);

	bool isEnable() const;

	void setChecked(bool checked);

	bool isChecked() const;

	void add(MenuItem* item);

	void remove(MenuItem* item);

	void removeAll();

	int count() const;

	Ref< MenuItem > get(int index);

private:
	Command m_command;
	std::wstring m_text;
	bool m_checkBox;
	Ref< Bitmap > m_image;
	bool m_enable;
	bool m_checked;
	RefArray< MenuItem > m_items;
};

	}
}

#endif	// traktor_ui_MenuItem_H
