#ifndef traktor_wbr_TreeView_H
#define traktor_wbr_TreeView_H

#include "Core/RefArray.h"
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

class TreeViewItem;

class T_DLLCLASS TreeView : public Widget
{
	T_RTTI_CLASS;

public:
	TreeView();

	TreeViewItem* addChild(TreeViewItem* child);

	void removeChild(TreeViewItem* child);

	const RefArray< TreeViewItem >& getChildren() const;

	virtual void build(html::Element* parent) const override;

	virtual void consume(int32_t senderId, int32_t action) override;

private:
	RefArray< TreeViewItem > m_children;
};

	}
}

#endif	// traktor_wbr_TreeView_H
