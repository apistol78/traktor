#ifndef traktor_ui_custom_LogList_H
#define traktor_ui_custom_LogList_H

#include <list>
#include "Core/Thread/Semaphore.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;

	namespace ui
	{

class Bitmap;
class ScrollBar;

		namespace custom
		{

/*! \brief Log list control.
 * \ingroup UIC
 */
class T_DLLCLASS LogList : public Widget
{
	T_RTTI_CLASS;

public:
	enum LogLevel
	{
		LvInfo		= 1 << 0,
		LvWarning	= 1 << 1,
		LvError		= 1 << 2,
		LvDebug		= 1 << 3
	};

	struct ISymbolLookup
	{
		virtual bool lookupLogSymbol(const Guid& symbolId, std::wstring& outSymbol) const = 0;
	};

	LogList();

	bool create(Widget* parent, int style, const ISymbolLookup* lookup);

	void add(LogLevel level, const std::wstring& text);

	void removeAll();

	void setFilter(uint32_t filter);

	uint32_t getFilter() const;

	bool copyLog(uint32_t filter = ~0U);

	virtual Size getPreferedSize() const;

private:
	struct Entry
	{
		uint32_t threadId;
		LogLevel logLevel;
		std::wstring logText;
	};

	typedef std::list< Entry > log_list_t;

	const ISymbolLookup* m_lookup;
	Ref< ScrollBar > m_scrollBar;
	Ref< Bitmap > m_icons;
	log_list_t m_pending;
	Semaphore m_pendingLock;
	log_list_t m_logFull;
	log_list_t m_logFiltered;
	std::map< uint32_t, uint32_t > m_threadIndices;
	int32_t m_itemHeight;
	uint32_t m_filter;
	uint32_t m_nextThreadIndex;

	void updateScrollBar();

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventScroll(ScrollEvent* event);

	void eventTimer(TimerEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_LogList_H
