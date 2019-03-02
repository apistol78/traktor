#pragma once

#include "Ui/Sequencer/Key.h"

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

/*! \brief Sequencer tick.
 * \ingroup UI
 */
class T_DLLCLASS Tick : public Key
{
	T_RTTI_CLASS;

public:
	Tick(int time, bool movable);

	void setTime(int time);

	int getTime() const;

	virtual void move(int offset) override;

	virtual void getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const override;

	virtual void paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset) override;

private:
	int m_time;
	bool m_movable;
};

	}
}

