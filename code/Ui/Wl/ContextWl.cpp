/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cmath>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include "Core/Assert.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/KeyDownEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/KeyUpEvent.h"
#include "Ui/Events/MouseButtonDownEvent.h"
#include "Ui/Events/MouseButtonUpEvent.h"
#include "Ui/Events/MouseMoveEvent.h"
#include "Ui/Wl/ContextWl.h"
#include "Ui/Wl/Timers.h"
#include "Ui/Wl/UtilitiesWl.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ContextWl", ContextWl, Object)

Timer ContextWl::ms_timer;

// Registry listener
static const wl_registry_listener s_registryListener = {
	ContextWl::registryGlobal,
	ContextWl::registryGlobalRemove
};

// Seat listener
static const wl_seat_listener s_seatListener = {
	ContextWl::seatCapabilities,
	ContextWl::seatName
};

// Pointer listener
static const wl_pointer_listener s_pointerListener = {
	ContextWl::pointerEnter,
	ContextWl::pointerLeave,
	ContextWl::pointerMotion,
	ContextWl::pointerButton,
	ContextWl::pointerAxis,
	ContextWl::pointerFrame,
	ContextWl::pointerAxisSource,
	ContextWl::pointerAxisStop,
	ContextWl::pointerAxisDiscrete
};

// Keyboard listener
static const wl_keyboard_listener s_keyboardListener = {
	ContextWl::keyboardKeymap,
	ContextWl::keyboardEnter,
	ContextWl::keyboardLeave,
	ContextWl::keyboardKey,
	ContextWl::keyboardModifiers,
	ContextWl::keyboardRepeatInfo
};

// XDG WM base listener
static const xdg_wm_base_listener s_xdgWmBaseListener = {
	ContextWl::xdgWmBasePing
};

// Output listener
static const wl_output_listener s_outputListener = {
	ContextWl::outputGeometry,
	ContextWl::outputMode,
	ContextWl::outputDone,
	ContextWl::outputScale
};

// Data device listener
static const wl_data_device_listener s_dataDeviceListener = {
	ContextWl::dataDeviceDataOffer,
	ContextWl::dataDeviceEnter,
	ContextWl::dataDeviceLeave,
	ContextWl::dataDeviceMotion,
	ContextWl::dataDeviceDrop,
	ContextWl::dataDeviceSelection
};

// Data offer listener - all callbacks must be non-null to be safe when data_offer
// proxy is at version >= 3 (some compositors may send source_actions/action).
static void dataOfferNoop2(void* data, wl_data_offer* offer, uint32_t value) {}

static const wl_data_offer_listener s_dataOfferListener = {
	ContextWl::dataOfferOffer,
	dataOfferNoop2,
	dataOfferNoop2
};

// Re-express a surface-local point in another widget's local space, via the
// shared toplevel.  Used to fix up coordinates during capture, since Wayland
// has no protocol equivalent of XGrabPointer to redirect events.
static void translateBetweenWidgets(const WidgetData* from, const WidgetData* to, double& x, double& y)
{
	if (!from || !to || from == to)
		return;

	for (const WidgetData* w = from; w != nullptr && !w->topLevel; w = w->parent)
	{
		x += w->posX;
		y += w->posY;
	}
	for (const WidgetData* w = to; w != nullptr && !w->topLevel; w = w->parent)
	{
		x -= w->posX;
		y -= w->posY;
	}
}

// libdecor interface
static void libdecorError(libdecor* context, libdecor_error error, const char* message)
{
	log::error << L"libdecor error: " << mbstows(message) << Endl;
}

static libdecor_interface s_libdecorInterface = {
	libdecorError
};

ContextWl::ContextWl()
{
}

ContextWl::~ContextWl()
{
	if (m_xkbState)
		xkb_state_unref(m_xkbState);
	if (m_xkbKeymap)
		xkb_keymap_unref(m_xkbKeymap);
	if (m_xkbContext)
		xkb_context_unref(m_xkbContext);

	if (m_cursorSurface)
		wl_surface_destroy(m_cursorSurface);
	if (m_cursorTheme)
		wl_cursor_theme_destroy(m_cursorTheme);
	if (m_pointer)
		wl_pointer_destroy(m_pointer);
	if (m_keyboard)
		wl_keyboard_destroy(m_keyboard);

	if (m_pendingOffer)
		wl_data_offer_destroy(m_pendingOffer);
	if (m_selectionOffer)
		wl_data_offer_destroy(m_selectionOffer);
	if (m_dataDevice)
		wl_data_device_destroy(m_dataDevice);
	if (m_dataDeviceManager)
		wl_data_device_manager_destroy(m_dataDeviceManager);

	if (m_libdecor)
		libdecor_unref(m_libdecor);

	if (m_display)
		wl_display_disconnect(m_display);
}

bool ContextWl::initialize()
{
	// Derive app id from the executable basename.  Wayland has no per-window
	// icon protocol in xdg-shell; the compositor maps app_id to an installed
	// .desktop file and reads Icon= from it, so the executable name must
	// match the basename of <app>.desktop for the icon to appear.
	const std::wstring appId = OS::getInstance().getExecutable().getFileName();
	log::info << L"Application identifier: " << appId << Endl;
	m_appId = wstombs(appId);

	m_display = wl_display_connect(nullptr);
	if (!m_display)
	{
		log::error << L"Failed to connect to Wayland display." << Endl;
		return false;
	}

	m_xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if (!m_xkbContext)
	{
		log::error << L"Failed to create xkb context." << Endl;
		return false;
	}

	m_registry = wl_display_get_registry(m_display);
	wl_registry_add_listener(m_registry, &s_registryListener, this);

	// First roundtrip to get globals.
	wl_display_roundtrip(m_display);
	// Second roundtrip to process globals.
	wl_display_roundtrip(m_display);

	if (!m_compositor || !m_xdgWmBase)
	{
		log::error << L"Wayland compositor missing required interfaces." << Endl;
		return false;
	}

	// Create data device for clipboard support.
	if (m_dataDeviceManager && m_seat)
	{
		m_dataDevice = wl_data_device_manager_get_data_device(m_dataDeviceManager, m_seat);
		wl_data_device_add_listener(m_dataDevice, &s_dataDeviceListener, this);
		wl_display_roundtrip(m_display);
	}

	// Create libdecor context for window decorations.
	m_libdecor = libdecor_new(m_display, &s_libdecorInterface);
	if (!m_libdecor)
	{
		log::error << L"Failed to create libdecor context." << Endl;
		return false;
	}

	// Create cursor theme and surface.
	m_cursorTheme = wl_cursor_theme_load(nullptr, 24, m_shm);
	if (m_cursorTheme)
		m_cursorSurface = wl_compositor_create_surface(m_compositor);

	return true;
}

void ContextWl::bind(WidgetData* widget, int32_t eventType, const std::function< void(WlEvent& e) >& fn)
{
	auto& b = m_bindings[widget->surface];
	b.widget = widget;
	b.fns[eventType] = fn;
}

void ContextWl::bindRenderSurface(WidgetData* widget)
{
	auto& b = m_bindings[widget->renderSurface];
	b.widget = widget;
	b.fns = m_bindings[widget->surface].fns;
}

void ContextWl::pushPopup(WidgetData* widget)
{
	m_popupStack.push_back(widget);
}

void ContextWl::destroyPopupsAbove(WidgetData* widget)
{
	// Find our position in the stack.
	auto it = std::find(m_popupStack.begin(), m_popupStack.end(), widget);
	if (it == m_popupStack.end())
		return;

	// Destroy all popups above us in reverse order (LIFO),
	// as required by xdg-shell.
	for (auto top = m_popupStack.end(); top != it + 1; )
	{
		--top;
		WidgetData* wd = *top;
		if (wd->xdgPopup)
		{
			xdg_popup_destroy(wd->xdgPopup);
			wd->xdgPopup = nullptr;
		}
		if (wd->xdgSurface && !wd->frame)
		{
			xdg_surface_destroy(wd->xdgSurface);
			wd->xdgSurface = nullptr;
		}
	}

	// Remove everything from our position onward.
	m_popupStack.erase(it, m_popupStack.end());
}

void ContextWl::unbind(WidgetData* widget)
{
	auto it = std::find(m_modal.begin(), m_modal.end(), widget);
	if (it != m_modal.end())
		m_modal.erase(it);

	if (widget->renderSurface)
		m_bindings.remove(widget->renderSurface);

	const bool erased = m_bindings.remove(widget->surface);
	T_FATAL_ASSERT(erased);

	if (m_grabbed == widget)
		m_grabbed = nullptr;

	if (m_pointerFocus == widget)
		m_pointerFocus = nullptr;

	if (m_keyboardFocus == widget)
		m_keyboardFocus = nullptr;

	if (m_internalFocus == widget)
		m_internalFocus = nullptr;

	// Remove from pending expose list.
	auto pe = std::find(m_pendingExposes.begin(), m_pendingExposes.end(), widget);
	if (pe != m_pendingExposes.end())
		m_pendingExposes.erase(pe);
}

void ContextWl::pushModal(WidgetData* widget)
{
	m_modal.push_back(widget);
}

void ContextWl::popModal(WidgetData* widget)
{
	T_FATAL_ASSERT(!m_modal.empty());
	T_FATAL_ASSERT(m_modal.back() == widget);
	m_modal.pop_back();
}

void ContextWl::grab(WidgetData* widget)
{
	T_FATAL_ASSERT(!widget->grabbed);

	if (m_grabbed)
	{
		m_grabbed->grabbed = false;
		m_grabbed = nullptr;
	}

	widget->grabbed = true;
	m_grabbed = widget;
}

void ContextWl::ungrab(WidgetData* widget)
{
	T_FATAL_ASSERT(widget->grabbed);
	T_FATAL_ASSERT(widget == m_grabbed);

	widget->grabbed = false;
	m_grabbed = nullptr;
}

void ContextWl::dispatch(WlEvent& e)
{
	if (!e.surface)
		return;

	// Events that bypass enable/modal checks: lifecycle and pointer
	// enter/leave (the latter so cursor management still works on
	// disabled or non-modal widgets).
	const bool always =
		e.type == WlEvtFocusIn ||
		e.type == WlEvtFocusOut ||
		e.type == WlEvtConfigure ||
		e.type == WlEvtClose ||
		e.type == WlEvtExpose ||
		e.type == WlEvtPointerEnter ||
		e.type == WlEvtPointerLeave;

	dispatch(e.surface, e.type, always, e);
}

void ContextWl::enqueueEvent(const WlEvent& e)
{
	m_eventQueue.push_back(e);
}

bool ContextWl::processEventQueue(EventSubject* owner)
{
	if (m_eventQueue.empty())
		return false;

	AlignedVector< WlEvent > queue;
	std::swap(queue, m_eventQueue);

	for (auto& e : queue)
	{
		if (!preTranslateEvent(owner, e))
			dispatch(e);
	}
	return true;
}

bool ContextWl::preTranslateEvent(EventSubject* owner, WlEvent& e)
{
	if (anyGrabbed())
		return false;

	if (e.type == WlEvtPointerMotion)
	{
		MouseMoveEvent mouseMoveEvent(
			owner,
			e.buttons,
			Point((int)e.pointerX, (int)e.pointerY)
		);
		owner->raiseEvent(&mouseMoveEvent);
	}
	else if (e.type == WlEvtPointerButton)
	{
		const int32_t button = translateMouseButton(e.button);

		if (e.buttonState == WL_POINTER_BUTTON_STATE_PRESSED)
		{
			MouseButtonDownEvent mouseButtonDownEvent(
				owner,
				button,
				Point((int)e.pointerX, (int)e.pointerY)
			);
			owner->raiseEvent(&mouseButtonDownEvent);
		}
		else
		{
			MouseButtonUpEvent mouseButtonUpEvent(
				owner,
				button,
				Point((int)e.pointerX, (int)e.pointerY)
			);
			owner->raiseEvent(&mouseButtonUpEvent);
		}
	}
	else if (e.type == WlEvtKeyboardKey)
	{
		if (!m_xkbState)
			return false;

		const uint32_t keycode = e.key + 8;
		const xkb_keysym_t sym = xkb_state_key_get_one_sym(m_xkbState, keycode);
		VirtualKey vk = translateToVirtualKey(sym);

		bool consumed = false;

		if (e.keyState == WL_KEYBOARD_KEY_STATE_PRESSED)
		{
			if (vk != VkNull)
			{
				KeyDownEvent keyDownEvent(owner, vk, e.key, 0);
				owner->raiseEvent(&keyDownEvent);
				consumed |= keyDownEvent.consumed();
			}

			char buf[8] = { 0 };
			int n = xkb_state_key_get_utf8(m_xkbState, keycode, buf, sizeof(buf));
			if (n > 0)
			{
				wchar_t wch = 0;
				if (Utf8Encoding().translate((const uint8_t*)buf, n, wch) > 0)
				{
					KeyEvent keyEvent(owner, vk, e.key, wch);
					owner->raiseEvent(&keyEvent);
					consumed |= keyEvent.consumed();
				}
			}
		}
		else
		{
			if (vk != VkNull)
			{
				KeyUpEvent keyUpEvent(owner, vk, e.key, 0, false);
				owner->raiseEvent(&keyUpEvent);
				consumed |= keyUpEvent.consumed();
			}
		}

		return consumed;
	}

	return false;
}

void ContextWl::queueExpose(WidgetData* widget)
{
	// Avoid duplicates.
	for (auto* w : m_pendingExposes)
	{
		if (w == widget)
			return;
	}
	m_pendingExposes.push_back(widget);
}

void ContextWl::queueExposeRecursive(WidgetData* widget)
{
	if (widget == nullptr || !widget->visible)
		return;

	queueExpose(widget);

	for (auto* child : widget->children)
		queueExposeRecursive(child);
}

bool ContextWl::processPendingExposes()
{
	if (m_pendingExposes.empty())
		return false;

	// Copy and clear to handle reentrant calls from draw.
	AlignedVector< WidgetData* > pending;
	std::swap(pending, m_pendingExposes);

	for (auto* widget : pending)
	{
		WlEvent e;
		e.type = WlEvtExpose;
		e.surface = widget->surface;
		dispatch(e);
	}
	return true;
}

Rect ContextWl::computeVisibleRect(const WidgetData* widget)
{
	// Toplevel widgets are not clipped against anything.
	if (widget->topLevel || widget->parent == nullptr)
		return Rect(0, 0, widget->width, widget->height);

	int32_t vLeft = 0;
	int32_t vTop = 0;
	int32_t vRight = widget->width;
	int32_t vBottom = widget->height;

	// Widget's top-left in the current ancestor's local coords.
	int32_t accLeft = widget->posX;
	int32_t accTop = widget->posY;

	for (const WidgetData* anc = widget->parent; anc != nullptr; anc = anc->parent)
	{
		// Intersection of widget rect with ancestor inner rect (in ancestor local).
		const int32_t iLeft = std::max< int32_t >(accLeft, 0);
		const int32_t iTop = std::max< int32_t >(accTop, 0);
		const int32_t iRight = std::min< int32_t >(accLeft + widget->width, anc->width);
		const int32_t iBottom = std::min< int32_t >(accTop + widget->height, anc->height);

		// Convert intersection back to widget local: subtract widget pos in this ancestor.
		vLeft = std::max< int32_t >(vLeft, iLeft - accLeft);
		vTop = std::max< int32_t >(vTop, iTop - accTop);
		vRight = std::min< int32_t >(vRight, iRight - accLeft);
		vBottom = std::min< int32_t >(vBottom, iBottom - accTop);

		if (vLeft >= vRight || vTop >= vBottom)
			return Rect(0, 0, 0, 0);

		if (anc->topLevel || anc->parent == nullptr)
			break;

		// Step up: widget pos in grandparent = widget pos in ancestor + ancestor pos.
		accLeft += anc->posX;
		accTop += anc->posY;
	}

	return Rect(vLeft, vTop, vRight, vBottom);
}

void ContextWl::applyClip(WidgetData* widget) const
{
	// Toplevels manage their own buffer; no clipping path.
	if (widget->topLevel || widget->subsurface == nullptr)
		return;

	const Rect visible = computeVisibleRect(widget);

	const int32_t vw = visible.getWidth();
	const int32_t vh = visible.getHeight();

	if (vw <= 0 || vh <= 0)
	{
		// Fully clipped — detach buffer so nothing is shown. m_data.mapped is
		// left as-is so a redraw rebinds the buffer once the widget becomes
		// visible again (e.g. parent grows back).
		if (widget->mapped)
		{
			wl_surface_attach(widget->surface, nullptr, 0, 0);
			wl_surface_commit(widget->surface);
		}
		if (widget->renderSurface)
		{
			wl_surface_attach(widget->renderSurface, nullptr, 0, 0);
			wl_surface_commit(widget->renderSurface);
		}
		return;
	}

	// Shift the subsurface origin by the amount we cropped from the top-left.
	wl_subsurface_set_position(
		widget->subsurface,
		toLogical(widget->posX + visible.left),
		toLogical(widget->posY + visible.top)
	);

	const int32_t dw = std::max< int32_t >(toLogical(vw), 1);
	const int32_t dh = std::max< int32_t >(toLogical(vh), 1);

	// The wp_viewport source rectangle is in the surface content-area space, which is
	// buffer_size / buffer_scale.
	//  - Legacy integer mode sets buffer_scale = output scale, so the content area is
	//    logical and the source must be logical too (device pixels would overflow it on
	//    HiDPI and raise wp_viewport out_of_buffer, leaving the surface unmapped).
	//  - Fractional mode sets buffer_scale = 1, so the content area is the full device
	//    buffer; the source is in device pixels and the destination alone downscales to
	//    the logical size.
	int32_t srcX, srcY, srcW, srcH;
	if (m_fractionalScaling)
	{
		srcX = visible.left;
		srcY = visible.top;
		srcW = vw;
		srcH = vh;
	}
	else
	{
		srcX = toLogical(visible.left);
		srcY = toLogical(visible.top);
		srcW = dw;
		srcH = dh;
	}

	if (widget->viewport != nullptr)
	{
		wp_viewport_set_source(
			widget->viewport,
			wl_fixed_from_int(srcX),
			wl_fixed_from_int(srcY),
			wl_fixed_from_int(srcW),
			wl_fixed_from_int(srcH)
		);
		wp_viewport_set_destination(widget->viewport, dw, dh);
	}

	if (widget->renderViewport != nullptr)
	{
		// The render subsurface lives at (0, 0) of the widget surface, so the
		// same visible rect crops it identically — output aligns with the
		// cropped cairo content.
		wp_viewport_set_source(
			widget->renderViewport,
			wl_fixed_from_int(srcX),
			wl_fixed_from_int(srcY),
			wl_fixed_from_int(srcW),
			wl_fixed_from_int(srcH)
		);
		wp_viewport_set_destination(widget->renderViewport, dw, dh);
	}
}

void ContextWl::applyClipRecursive(WidgetData* widget) const
{
	applyClip(widget);
	for (auto* child : widget->children)
		applyClipRecursive(child);
}

void ContextWl::setInternalFocus(WidgetData* widget)
{
	if (widget == m_internalFocus)
		return;

	// Send FocusOut to the previously focused widget.
	if (m_internalFocus)
	{
		WlEvent e;
		e.type = WlEvtFocusOut;
		e.surface = m_internalFocus->surface;
		dispatch(e);
	}

	m_internalFocus = widget;

	// Send FocusIn to the newly focused widget.
	if (m_internalFocus)
	{
		WlEvent e;
		e.type = WlEvtFocusIn;
		e.surface = m_internalFocus->surface;
		dispatch(e);
	}
}

int32_t ContextWl::getSystemDPI() const
{
	return m_dpi;
}

int32_t ContextWl::toLogical(int32_t device) const
{
	if (m_fractionalScaling)
		return (int32_t)std::lround(device / m_scale);
	// Legacy integer path: floor so a viewport source rectangle never exceeds the
	// buffer content area (buffer_size / buffer_scale).
	return (m_outputScale > 1) ? device / m_outputScale : device;
}

int32_t ContextWl::toDevice(int32_t logical) const
{
	if (m_fractionalScaling)
		return (int32_t)std::lround(logical * m_scale);
	return logical * m_outputScale;
}

bool ContextWl::applyFractionalScale(uint32_t scale120)
{
	if (scale120 == 0)
		return false;
	const float s = scale120 / 120.0f;
	if (m_fractionalScaling && std::abs(s - m_scale) < 0.001f)
		return false;
	m_scale = s;
	m_fractionalScaling = true;
	m_dpi = (int32_t)std::lround(96.0f * s);
	return true;
}

void ContextWl::dispatch(wl_surface* surface, int32_t eventType, bool always, WlEvent& e)
{
	auto b = m_bindings.find(surface);
	if (b == m_bindings.end())
		return;

	const auto& binding = b->second;
	T_FATAL_ASSERT(binding.widget != nullptr);

	if (!always && binding.widget != m_grabbed)
	{
		for (const WidgetData* w = binding.widget; w != nullptr; w = w->parent)
		{
			if (!w->enable)
				return;
		}

		if (!m_modal.empty())
		{
			bool p = false;
			for (const WidgetData* w = binding.widget; w != nullptr; w = w->parent)
			{
				if (w == m_modal.back())
				{
					p = true;
					break;
				}
			}
			if (!p)
				return;
		}
	}

	auto d = binding.fns.find(eventType);
	if (d != binding.fns.end())
	{
		std::function< void(WlEvent& e) > fn = d->second;
		fn(e);
	}
}

// Registry
void ContextWl::registryGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	if (std::strcmp(interface, wl_compositor_interface.name) == 0)
		ctx->m_compositor = static_cast< wl_compositor* >(wl_registry_bind(registry, name, &wl_compositor_interface, 4));
	else if (std::strcmp(interface, wl_subcompositor_interface.name) == 0)
		ctx->m_subcompositor = static_cast< wl_subcompositor* >(wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
	else if (std::strcmp(interface, wp_viewporter_interface.name) == 0)
		ctx->m_viewporter = static_cast< wp_viewporter* >(wl_registry_bind(registry, name, &wp_viewporter_interface, 1));
	else if (std::strcmp(interface, wp_fractional_scale_manager_v1_interface.name) == 0)
		ctx->m_fractionalScaleManager = static_cast< wp_fractional_scale_manager_v1* >(wl_registry_bind(registry, name, &wp_fractional_scale_manager_v1_interface, 1));
	else if (std::strcmp(interface, wl_shm_interface.name) == 0)
		ctx->m_shm = static_cast< wl_shm* >(wl_registry_bind(registry, name, &wl_shm_interface, 1));
	else if (std::strcmp(interface, xdg_wm_base_interface.name) == 0)
	{
		ctx->m_xdgWmBase = static_cast< xdg_wm_base* >(wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));
		xdg_wm_base_add_listener(ctx->m_xdgWmBase, &s_xdgWmBaseListener, ctx);
	}
	else if (std::strcmp(interface, wl_seat_interface.name) == 0)
	{
		ctx->m_seat = static_cast< wl_seat* >(wl_registry_bind(registry, name, &wl_seat_interface, 5));
		wl_seat_add_listener(ctx->m_seat, &s_seatListener, ctx);
	}
	else if (std::strcmp(interface, wl_output_interface.name) == 0)
	{
		ctx->m_output = static_cast< wl_output* >(wl_registry_bind(registry, name, &wl_output_interface, 2));
		wl_output_add_listener(ctx->m_output, &s_outputListener, ctx);
	}
	else if (std::strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
	{
		ctx->m_decorationManager = static_cast< zxdg_decoration_manager_v1* >(
			wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1));
	}
	else if (std::strcmp(interface, wl_data_device_manager_interface.name) == 0)
	{
		ctx->m_dataDeviceManager = static_cast< wl_data_device_manager* >(
			wl_registry_bind(registry, name, &wl_data_device_manager_interface, 1));
	}
}

void ContextWl::registryGlobalRemove(void* data, wl_registry* registry, uint32_t name)
{
}

// Seat
void ContextWl::seatCapabilities(void* data, wl_seat* seat, uint32_t caps)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	if ((caps & WL_SEAT_CAPABILITY_POINTER) && !ctx->m_pointer)
	{
		ctx->m_pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(ctx->m_pointer, &s_pointerListener, ctx);
	}

	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !ctx->m_keyboard)
	{
		ctx->m_keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_add_listener(ctx->m_keyboard, &s_keyboardListener, ctx);
	}
}

void ContextWl::seatName(void* data, wl_seat* seat, const char* name)
{
}

// Pointer
void ContextWl::pointerEnter(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	ctx->m_pointerSerial = serial;

	double px = wl_fixed_to_double(sx) * ctx->getScale();
	double py = wl_fixed_to_double(sy) * ctx->getScale();

	WidgetData* enteringWidget = nullptr;
	auto b = ctx->m_bindings.find(surface);
	if (b != ctx->m_bindings.end())
	{
		enteringWidget = b->second.widget;
		ctx->m_pointerFocus = enteringWidget;
	}

	// Keep cached coordinates in the grabbed widget's space so a button event
	// arriving before the next motion still sees the correct position.
	if (ctx->m_grabbed && enteringWidget && enteringWidget != ctx->m_grabbed)
		translateBetweenWidgets(enteringWidget, ctx->m_grabbed, px, py);

	ctx->m_pointerX = px;
	ctx->m_pointerY = py;

	WlEvent e;
	e.type = WlEvtPointerEnter;
	e.surface = surface;
	e.pointerX = ctx->m_pointerX;
	e.pointerY = ctx->m_pointerY;
	e.serial = serial;
	e.stamp = ms_timer.getElapsedTime();
	ctx->enqueueEvent(e);
}

void ContextWl::pointerLeave(void* data, wl_pointer* pointer, uint32_t serial, wl_surface* surface)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	ctx->m_pointerSerial = serial;

	WlEvent e;
	e.type = WlEvtPointerLeave;
	e.surface = surface;
	e.serial = serial;
	e.stamp = ms_timer.getElapsedTime();
	ctx->enqueueEvent(e);

	if (ctx->m_pointerFocus)
	{
		auto b = ctx->m_bindings.find(surface);
		if (b != ctx->m_bindings.end() && b->second.widget == ctx->m_pointerFocus)
			ctx->m_pointerFocus = nullptr;
	}
}

void ContextWl::pointerMotion(void* data, wl_pointer* pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	WidgetData* target = ctx->m_pointerFocus;
	if (ctx->m_grabbed)
		target = ctx->m_grabbed;

	if (!target || !target->surface)
		return;

	double px = wl_fixed_to_double(sx) * ctx->getScale();
	double py = wl_fixed_to_double(sy) * ctx->getScale();

	// During capture, motion can come in via a different surface (the parent
	// or a sibling) once the cursor leaves the grabbed widget's bounds —
	// rebase the coordinates into the grabbed widget's space.
	if (ctx->m_grabbed && ctx->m_pointerFocus && ctx->m_pointerFocus != ctx->m_grabbed)
		translateBetweenWidgets(ctx->m_pointerFocus, ctx->m_grabbed, px, py);

	ctx->m_pointerX = px;
	ctx->m_pointerY = py;

	WlEvent e;
	e.type = WlEvtPointerMotion;
	e.surface = target->surface;
	e.pointerX = ctx->m_pointerX;
	e.pointerY = ctx->m_pointerY;
	e.buttons = ctx->m_buttonMask;
	e.stamp = ms_timer.getElapsedTime();
	ctx->enqueueEvent(e);
}

void ContextWl::pointerButton(void* data, wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	ctx->m_pointerSerial = serial;
	ctx->m_inputSerial = serial;

	// Save button-press serials separately — Mutter only accepts press
	// serials for xdg_popup_grab (not release or enter serials).
	if (state == WL_POINTER_BUTTON_STATE_PRESSED)
		ctx->m_grabSerial = serial;

	// Maintain running button mask (MbtLeft/MbtMiddle/MbtRight).
	const int32_t mbt = translateMouseButton(button);
	if (state == WL_POINTER_BUTTON_STATE_PRESSED)
		ctx->m_buttonMask |= mbt;
	else
		ctx->m_buttonMask &= ~mbt;

	wl_surface* surface = ctx->m_pointerFocus ? ctx->m_pointerFocus->surface : nullptr;
	if (ctx->m_grabbed)
		surface = ctx->m_grabbed->surface;

	if (!surface)
		return;

	WlEvent e;
	e.type = WlEvtPointerButton;
	e.surface = surface;
	e.pointerX = ctx->m_pointerX;
	e.pointerY = ctx->m_pointerY;
	e.button = button;
	e.buttonState = state;
	e.serial = serial;
	e.stamp = ms_timer.getElapsedTime();
	ctx->enqueueEvent(e);
}

void ContextWl::pointerAxis(void* data, wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	wl_surface* surface = ctx->m_pointerFocus ? ctx->m_pointerFocus->surface : nullptr;
	if (!surface)
		return;

	WlEvent e;
	e.type = WlEvtPointerAxis;
	e.surface = surface;
	e.pointerX = ctx->m_pointerX;
	e.pointerY = ctx->m_pointerY;
	e.axisType = axis;
	e.axisValue = wl_fixed_to_double(value);
	e.stamp = ms_timer.getElapsedTime();
	ctx->enqueueEvent(e);
}

void ContextWl::pointerFrame(void* data, wl_pointer* pointer) {}
void ContextWl::pointerAxisSource(void* data, wl_pointer* pointer, uint32_t source) {}
void ContextWl::pointerAxisStop(void* data, wl_pointer* pointer, uint32_t time, uint32_t axis) {}
void ContextWl::pointerAxisDiscrete(void* data, wl_pointer* pointer, uint32_t axis, int32_t discrete) {}

// Keyboard
void ContextWl::keyboardKeymap(void* data, wl_keyboard* keyboard, uint32_t format, int32_t fd, uint32_t size)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
	{
		close(fd);
		return;
	}

	char* mapStr = static_cast< char* >(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
	if (mapStr == MAP_FAILED)
	{
		close(fd);
		return;
	}

	if (ctx->m_xkbState)
	{
		xkb_state_unref(ctx->m_xkbState);
		ctx->m_xkbState = nullptr;
	}
	if (ctx->m_xkbKeymap)
	{
		xkb_keymap_unref(ctx->m_xkbKeymap);
		ctx->m_xkbKeymap = nullptr;
	}

	ctx->m_xkbKeymap = xkb_keymap_new_from_string(ctx->m_xkbContext, mapStr, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(mapStr, size);
	close(fd);

	if (ctx->m_xkbKeymap)
		ctx->m_xkbState = xkb_state_new(ctx->m_xkbKeymap);
}

void ContextWl::keyboardEnter(void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface, wl_array* keys)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	auto b = ctx->m_bindings.find(surface);
	if (b != ctx->m_bindings.end())
	{
		ctx->m_keyboardFocus = b->second.widget;
	}
	else
	{
		// Surface not in bindings — libdecor CSD creates an internal
		// parent surface that we don't track. Fall back to the first
		// toplevel widget so keyboard routing still works.
		for (auto it = ctx->m_bindings.begin(); it != ctx->m_bindings.end(); ++it)
		{
			if (it->second.widget && it->second.widget->topLevel)
			{
				ctx->m_keyboardFocus = it->second.widget;
				break;
			}
		}
	}

	// If no child widget has internal focus, give it to the toplevel.
	if (!ctx->m_internalFocus && ctx->m_keyboardFocus)
		ctx->m_internalFocus = ctx->m_keyboardFocus;

	// Dispatch FocusIn to the internally focused widget.
	if (ctx->m_internalFocus)
	{
		WlEvent e;
		e.type = WlEvtFocusIn;
		e.surface = ctx->m_internalFocus->surface;
		e.serial = serial;
		e.stamp = ms_timer.getElapsedTime();
		ctx->enqueueEvent(e);
	}
}

void ContextWl::keyboardLeave(void* data, wl_keyboard* keyboard, uint32_t serial, wl_surface* surface)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	// Stop key repeat on focus loss.
	if (ctx->m_repeatTimer >= 0)
	{
		Timers::getInstance().unbind(ctx->m_repeatTimer);
		ctx->m_repeatTimer = -1;
	}

	// Dispatch FocusOut to the internally focused widget.
	if (ctx->m_internalFocus)
	{
		WlEvent e;
		e.type = WlEvtFocusOut;
		e.surface = ctx->m_internalFocus->surface;
		e.serial = serial;
		e.stamp = ms_timer.getElapsedTime();
		ctx->enqueueEvent(e);
	}

	ctx->m_keyboardFocus = nullptr;
	ctx->m_internalFocus = nullptr;
}

void ContextWl::keyboardKey(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	ctx->m_inputSerial = serial;

	// Stop any active key repeat.
	if (ctx->m_repeatTimer >= 0)
	{
		Timers::getInstance().unbind(ctx->m_repeatTimer);
		ctx->m_repeatTimer = -1;
	}

	// Route keyboard events to the internally focused widget,
	// falling back to the compositor-level keyboard focus toplevel.
	WidgetData* target = ctx->m_internalFocus ? ctx->m_internalFocus : ctx->m_keyboardFocus;
	if (!target)
		return;

	WlEvent e;
	e.type = WlEvtKeyboardKey;
	e.surface = target->surface;
	e.key = key;
	e.keyState = state;
	e.serial = serial;
	e.stamp = ms_timer.getElapsedTime();
	ctx->enqueueEvent(e);

	// Start key repeat for press events on keys that support it.
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED && ctx->m_repeatRate > 0 && ctx->m_xkbKeymap)
	{
		if (xkb_keymap_key_repeats(ctx->m_xkbKeymap, key + 8))
		{
			ctx->m_repeatKey = key;
			const int32_t intervalMs = 1000 / ctx->m_repeatRate;

			// First repeat fires after the delay; subsequent repeats
			// are re-armed inside the callback at the repeat interval.
			ctx->m_repeatTimer = Timers::getInstance().bind(ctx->m_repeatDelay, [ctx, key, serial, intervalMs]() {
				// Stop the delay timer and switch to interval timer.
				if (ctx->m_repeatTimer >= 0)
				{
					Timers::getInstance().unbind(ctx->m_repeatTimer);
					ctx->m_repeatTimer = -1;
				}

				// Emit the first repeat.
				WidgetData* t = ctx->m_internalFocus ? ctx->m_internalFocus : ctx->m_keyboardFocus;
				if (t)
				{
					WlEvent re;
					re.type = WlEvtKeyboardKey;
					re.surface = t->surface;
					re.key = key;
					re.keyState = WL_KEYBOARD_KEY_STATE_PRESSED;
					re.serial = serial;
					re.stamp = ms_timer.getElapsedTime();
					ctx->enqueueEvent(re);
				}

				// Arm the steady-state interval timer.
				ctx->m_repeatTimer = Timers::getInstance().bind(intervalMs, [ctx, key, serial]() {
					WidgetData* t = ctx->m_internalFocus ? ctx->m_internalFocus : ctx->m_keyboardFocus;
					if (!t)
						return;
					WlEvent re;
					re.type = WlEvtKeyboardKey;
					re.surface = t->surface;
					re.key = key;
					re.keyState = WL_KEYBOARD_KEY_STATE_PRESSED;
					re.serial = serial;
					re.stamp = ms_timer.getElapsedTime();
					ctx->enqueueEvent(re);
				});
			});
		}
	}
}

void ContextWl::keyboardModifiers(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t modsDepressed, uint32_t modsLatched, uint32_t modsLocked, uint32_t group)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	if (ctx->m_xkbState)
		xkb_state_update_mask(ctx->m_xkbState, modsDepressed, modsLatched, modsLocked, 0, 0, group);

	ctx->m_keyboardModifiers = modsDepressed;
}

void ContextWl::keyboardRepeatInfo(void* data, wl_keyboard* keyboard, int32_t rate, int32_t delay)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	ctx->m_repeatRate = rate;
	ctx->m_repeatDelay = delay;
}

// XDG WM base
void ContextWl::xdgWmBasePing(void* data, xdg_wm_base* wmBase, uint32_t serial)
{
	xdg_wm_base_pong(wmBase, serial);
}

// Output
void ContextWl::outputGeometry(void* data, wl_output* output, int32_t x, int32_t y, int32_t pw, int32_t ph, int32_t subpixel, const char* make, const char* model, int32_t transform)
{
}

void ContextWl::outputMode(void* data, wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	if (flags & WL_OUTPUT_MODE_CURRENT)
	{
		ctx->m_outputWidth = width;
		ctx->m_outputHeight = height;
	}
}

void ContextWl::outputDone(void* data, wl_output* output)
{
}

void ContextWl::outputScale(void* data, wl_output* output, int32_t factor)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	ctx->m_outputScale = factor;
	// In legacy (integer buffer_scale) mode the effective scale tracks the output
	// scale. When wp_fractional_scale_v1 is driving the scale, leave m_scale/m_dpi
	// alone — the fractional preferred scale is authoritative.
	if (!ctx->m_fractionalScaling)
	{
		ctx->m_scale = (float)factor;
		// Set effective DPI so dpi96() returns scale-adjusted values.
		// This is needed by code that rasterizes bitmaps/SVGs at the
		// correct physical resolution (e.g. StyleBitmap).
		ctx->m_dpi = 96 * factor;
	}
}

// Data device
void ContextWl::dataDeviceDataOffer(void* data, wl_data_device* device, wl_data_offer* offer)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	// A new offer was created; start collecting its MIME types.
	if (ctx->m_pendingOffer)
		wl_data_offer_destroy(ctx->m_pendingOffer);
	ctx->m_pendingOffer = offer;
	ctx->m_pendingMimeTypes.resize(0);
	wl_data_offer_add_listener(offer, &s_dataOfferListener, ctx);
}

void ContextWl::dataDeviceEnter(void* data, wl_data_device* device, uint32_t serial, wl_surface* surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer* offer)
{
	// Drag-and-drop enter — this offer belongs to a DnD session, not clipboard.
	// If the pending offer we've been collecting MIME types for is in fact
	// a DnD offer, discard it so it isn't confused with a selection.
	ContextWl* ctx = static_cast< ContextWl* >(data);
	if (ctx->m_pendingOffer && ctx->m_pendingOffer == offer)
	{
		wl_data_offer_destroy(ctx->m_pendingOffer);
		ctx->m_pendingOffer = nullptr;
		ctx->m_pendingMimeTypes.resize(0);
	}
}

void ContextWl::dataDeviceLeave(void* data, wl_data_device* device)
{
}

void ContextWl::dataDeviceMotion(void* data, wl_data_device* device, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
}

void ContextWl::dataDeviceDrop(void* data, wl_data_device* device)
{
}

void ContextWl::dataDeviceSelection(void* data, wl_data_device* device, wl_data_offer* offer)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);

	// Destroy previous selection offer.
	if (ctx->m_selectionOffer && ctx->m_selectionOffer != offer)
		wl_data_offer_destroy(ctx->m_selectionOffer);

	if (offer)
	{
		// Promote the pending offer to the current selection.
		ctx->m_selectionOffer = offer;
		ctx->m_selectionMimeTypes = ctx->m_pendingMimeTypes;
		ctx->m_pendingOffer = nullptr;
		ctx->m_pendingMimeTypes.resize(0);
	}
	else
	{
		ctx->m_selectionOffer = nullptr;
		ctx->m_selectionMimeTypes.resize(0);
	}
}

// Data offer
void ContextWl::dataOfferOffer(void* data, wl_data_offer* offer, const char* mimeType)
{
	ContextWl* ctx = static_cast< ContextWl* >(data);
	if (offer == ctx->m_pendingOffer)
		ctx->m_pendingMimeTypes.push_back(std::string(mimeType));
}

}
