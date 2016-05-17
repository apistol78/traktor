#ifndef traktor_amalgam_ProfilerDialog_H
#define traktor_amalgam_ProfilerDialog_H

#include "Ui/Dialog.h"

namespace traktor
{
	namespace amalgam
	{

class ProfilerControl;
class TargetConnection;

/*! \brief Visual profiler dialog.
 * \ingroup Amalgam
 */
class ProfilerDialog : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	ProfilerDialog(TargetConnection* connection);

	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE;

private:
	Ref< TargetConnection > m_connection;
	Ref< ProfilerControl > m_control;

	void eventClose(ui::CloseEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

	}
}

#endif	// traktor_amalgam_ProfilerDialog_H
