/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_DropListCell_H
#define traktor_amalgam_DropListCell_H

#include "Ui/Command.h"
#include "Ui/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace amalgam
	{

class HostEnumerator;
class TargetInstance;

/*! \brief
 * \ingroup Amalgam
 */
class DropListCell : public ui::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	DropListCell(HostEnumerator* hostEnumerator, TargetInstance* instance);

protected:
	virtual void mouseDown(ui::MouseButtonDownEvent* event, const ui::Point& position)  override final;

	virtual void paint(ui::Canvas& canvas, const ui::Rect& rect)  override final;

private:
	Ref< HostEnumerator > m_hostEnumerator;
	Ref< TargetInstance > m_instance;
	ui::Point m_menuPosition;
};

	}
}

#endif	// traktor_amalgam_DropListCell_H
