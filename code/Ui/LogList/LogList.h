/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Guid.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/StaticMap.h"
#include "Core/Thread/Semaphore.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class IBitmap;
class ScrollBar;

/*! Log list control.
 * \ingroup UI
 */
class T_DLLCLASS LogList : public Widget
{
	T_RTTI_CLASS;

public:
	enum LogLevel : uint8_t
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

	bool create(Widget* parent, uint32_t style, const ISymbolLookup* lookup);

	void add(uint32_t threadId, LogLevel level, const std::wstring& text);

	void removeAll();

	void setFilter(uint8_t filter);

	uint8_t getFilter() const;

	void selectLine(int32_t line);

	void showLine(int32_t line);

	bool copyLog(uint8_t filter = ~0);

	void forEachLine(const std::function< void(int32_t line, const std::wstring& text) >& fn) const;

	void forEachFilteredLine(const std::function< void(int32_t line, const std::wstring& text) >& fn) const;

	virtual Size getPreferredSize(const Size& hint) const override;

private:
	struct Entry
	{
		uint32_t threadId;
		LogLevel level;
		std::wstring text;
		Guid symbolId;
	};

	typedef AlignedVector< Entry > log_list_t;

	const ISymbolLookup* m_lookup = nullptr;
	Ref< ScrollBar > m_scrollBarH;
	Ref< ScrollBar > m_scrollBarV;
	Ref< IBitmap > m_icons;
	log_list_t m_pending;
	Semaphore m_pendingLock;
	log_list_t m_logFull;
	log_list_t m_logFiltered;
	uint32_t m_logCount[3] = { 0, 0, 0 };
	StaticMap< uint32_t, uint32_t, 128 > m_threadIndices;
	Unit m_itemHeight = 0_ut;
	int32_t m_maxLineWidth = 0;
	uint8_t m_filter = LvInfo | LvWarning | LvError;
	uint32_t m_nextThreadIndex = 0;
	int32_t m_selectedEntry = -1;

	void updateScrollBar();

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);

	void eventMouseButtonDown(MouseButtonDownEvent* event);

	void eventMouseDoubleClick(MouseDoubleClickEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);
    
    void eventTimer(TimerEvent* event);

	void eventScroll(ScrollEvent* event);
};

}
