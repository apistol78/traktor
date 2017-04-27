/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_TreeViewItem_H
#define traktor_ui_TreeViewItem_H

#include <list>
#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Ui/Associative.h"

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

/*! \brief Tree view item.
 * \ingroup UI
 */
class T_DLLCLASS TreeViewItem
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	virtual void setText(const std::wstring& text) = 0;

	virtual std::wstring getText() const = 0;

	virtual void setBold(bool bold) = 0;

	virtual bool isBold() const = 0;

	virtual void setImage(int image) = 0;

	virtual int getImage() const = 0;

	virtual void setExpandedImage(int selectedImage) = 0;

	virtual int getExpandedImage() const = 0;

	virtual bool isExpanded() const = 0;

	virtual void expand() = 0;

	virtual bool isCollapsed() const = 0;

	virtual void collapse() = 0;

	virtual bool isSelected() const = 0;

	virtual void select() = 0;

	virtual bool isVisible() const = 0;

	virtual void show() = 0;

	virtual void setEditable(bool editable) = 0;

	virtual bool isEditable() const = 0;

	virtual bool edit() = 0;

	virtual void sort(bool recursive) = 0;

	virtual Ref< TreeViewItem > getParent() const = 0;

	virtual bool hasChildren() const = 0;

	virtual int getChildren(RefArray< TreeViewItem >& outChildren) const = 0;

	Ref< TreeViewItem > findChild(const std::wstring& childPath);

	/*! \brief Get path to this item.
	 *
	 * Path is separated with / for each level.
	 */
	std::wstring getPath() const;

private:
	RefArray< TreeViewItem > m_children;
};

	}
}

#endif	// traktor_ui_TreeViewItem_H
