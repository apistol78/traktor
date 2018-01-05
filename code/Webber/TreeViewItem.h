#ifndef traktor_wbr_TreeViewItem_H
#define traktor_wbr_TreeViewItem_H

#include "Core/RefArray.h"
#include "Webber/Event.h"
#include "Webber/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEBBER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace wbr
	{

class T_DLLCLASS TreeViewItem : public Widget
{
	T_RTTI_CLASS;

public:
	TreeViewItem(const std::wstring& text);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setExpanded(bool expanded);

	bool isExpanded() const;

	TreeViewItem* addChild(TreeViewItem* child);

	void removeChild(TreeViewItem* child);

	const RefArray< TreeViewItem >& getChildren() const;

	Event& clicked();

	virtual void build(html::Element* parent) const override;

	virtual void consume(int32_t senderId, int32_t action) override;

private:
	std::wstring m_text;
	bool m_expanded;
	RefArray< TreeViewItem > m_children;
	Event m_clicked;
};

	}
}

#endif	// traktor_wbr_TreeViewItem_H
