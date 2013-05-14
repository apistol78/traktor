#ifndef traktor_ui_custom_Track_H
#define traktor_ui_custom_Track_H

#include "Ui/Custom/Sequencer/Key.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Sequencer range.
 * \ingroup UIC
 */
class T_DLLCLASS Track : public Key
{
	T_RTTI_CLASS;

public:
	Track(int start, int end);

	void setStart(int start);

	int getStart() const;

	void setEnd(int end);

	int getEnd() const;

	virtual void move(int offset);

	virtual void getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const;

	virtual void paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset);

private:
	int m_start;
	int m_end;
};

		}
	}
}

#endif	// traktor_ui_custom_Track_H
