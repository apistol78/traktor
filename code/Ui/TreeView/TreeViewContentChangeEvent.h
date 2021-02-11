#pragma once

#include <string>
#include "Ui/Events/ContentChangeEvent.h"

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

class TreeViewItem;

/*!
 * \ingroup UI
 */
class T_DLLCLASS TreeViewContentChangeEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;

public:
	TreeViewContentChangeEvent(EventSubject* sender, TreeViewItem* item, const std::wstring& originalText);

	TreeViewItem* getItem() const;

	const std::wstring& getOriginalText() const;

private:
	Ref< TreeViewItem > m_item;
	std::wstring m_originalText;
};

	}
}

