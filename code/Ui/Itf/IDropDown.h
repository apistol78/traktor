/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IDropDown_H
#define traktor_ui_IDropDown_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

/*! \brief DropDown interface.
 * \ingroup UI
 */
class IDropDown : public IWidget
{
public:
	virtual bool create(IWidget* parent, const std::wstring& text, int style) = 0;

	virtual int add(const std::wstring& item) = 0;

	virtual bool remove(int index) = 0;

	virtual void removeAll() = 0;

	virtual int count() const = 0;

	virtual void set(int index, const std::wstring& item) = 0;

	virtual std::wstring get(int index) const = 0;
	
	virtual void select(int index) = 0;

	virtual int getSelected() const = 0;
};

	}
}

#endif	// traktor_ui_IDropDown_H
