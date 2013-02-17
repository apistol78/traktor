#ifndef traktor_ui_custom_Marker_H
#define traktor_ui_custom_Marker_H

#include "Ui/Custom/Sequencer/Key.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Sequencer marker.
 * \ingroup UIC
 */
class T_DLLCLASS Marker : public Key
{
	T_RTTI_CLASS;

public:
	Marker(int time);

	void setTime(int time);

	int getTime() const;

	virtual void move(int offset);

	virtual void getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const;

	virtual void paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset);

private:
	int m_time;
};

		}
	}
}

#endif	// traktor_ui_custom_Marker_H
