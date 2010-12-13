#ifndef traktor_drone_PerforceChangeListDialog_H
#define traktor_drone_PerforceChangeListDialog_H

#include <Core/Ref.h>
#include <Core/RefArray.h>
#include <Ui/ConfigDialog.h>

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class GridView;

		}
	}

	namespace drone
	{

class PerforceChangeList;

class PerforceChangeListDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS

public:
	bool create(ui::Widget* parent, const std::wstring& text, const RefArray< PerforceChangeList >& changeLists);

	void getSelectedChangeLists(RefArray< PerforceChangeList >& outSelectedChangeLists) const;

private:
	Ref< ui::custom::GridView > m_changeLists;
};

	}
}

#endif	// traktor_drone_PerforceChangeListDialog_H
