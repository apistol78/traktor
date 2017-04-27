/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IListBox_H
#define traktor_ui_IListBox_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief ListBox interface.
 * \ingroup UI
 */
class IListBox : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;

	virtual int add(const std::wstring& item) = 0;

	virtual bool remove(int index) = 0;

	virtual void removeAll() = 0;

	virtual int count() const = 0;

	virtual void set(int index, const std::wstring& item) = 0;

	virtual std::wstring get(int index) const = 0;

	virtual void select(int index) = 0;

	virtual bool selected(int index) const = 0;

	virtual Rect getItemRect(int index) const = 0;
};

	}
}

#endif	// traktor_ui_IListBox_H
