/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/IRefCount.h"
#include "Core/Ref.h"
#include "Ui/Enums.h"
#include "Ui/Point.h"
#include "Ui/Rect.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Canvas;
class EventSubject;
class IWidget;

/*! Host routing paint and input to the virtual widgets of a native top-level
 * window; hides the implementation from the backends.
 * \ingroup UI
 */
class ITopLevelWidgetHost : public IRefCount
{
public:
	/*! Services the host requires from its native top-level peer. */
	class IPeer
	{
	public:
		virtual ~IPeer() {}

		virtual IWidget* getPeerWidget() = 0;

		/*! Start native timer; callback fired on UI thread until stopped. */
		virtual int32_t startHostTimer(int32_t interval, const std::function< void() >& fn) = 0;

		virtual void stopHostTimer(int32_t id) = 0;
	};

	/*! \name Dispatch from native top-level peer.
	 *
	 * Points are in top-level client coordinates, device pixels. Return
	 * value indicates the event was consumed.
	 */
	//@{

	virtual void paint(Canvas& canvas, const Rect& rcUpdate) = 0;

	virtual bool dispatchMouseButtonDown(int32_t button, const Point& pt) = 0;

	virtual bool dispatchMouseButtonUp(int32_t button, const Point& pt) = 0;

	virtual bool dispatchMouseDoubleClick(int32_t button, const Point& pt) = 0;

	virtual bool dispatchMouseMove(int32_t button, const Point& pt) = 0;

	virtual bool dispatchMouseWheel(int32_t rotation, const Point& pt) = 0;

	virtual bool dispatchKeyDown(VirtualKey vk, int32_t systemKey) = 0;

	virtual bool dispatchKeyUp(VirtualKey vk, int32_t systemKey) = 0;

	virtual bool dispatchKey(VirtualKey vk, int32_t systemKey, wchar_t ch) = 0;

	virtual void dispatchMouseTrack(bool entered, const Point& pt) = 0;

	virtual void dispatchFocus(bool gained) = 0;

	virtual void captureLost() = 0;

	//@}

	virtual Point getLastMousePosition() const = 0;
};

/*! Create host routing paint and input for a native top-level peer. */
T_DLLCLASS Ref< ITopLevelWidgetHost > createTopLevelWidgetHost(ITopLevelWidgetHost::IPeer* peer, EventSubject* owner);

}
