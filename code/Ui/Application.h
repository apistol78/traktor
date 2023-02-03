/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Enums.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/FocusEvent.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/Events/KeyDownEvent.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/KeyUpEvent.h"
#include "Ui/Itf/IEventLoop.h"
#include "Ui/Itf/IWidgetFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

}

namespace traktor::ui
{

class Clipboard;
class StyleSheet;

/*! User interface application.
 * \ingroup UI
 *
 * The Application singleton is used to register
 * which widget factory implementation to use.
 * It also provides system event handling and
 * clipboard access.
 */
class T_DLLCLASS Application : public EventSubject
{
	T_RTTI_CLASS;

public:
	Application();

	static Application* getInstance();

	/*! Initialize UI application.
	 *
	 * This must be called prior to any UI related
	 * calls are being made.
	 *
	 * \param widgetFactory System widget factory instance.
	 * \return True if application initialization succeeded.
	 */
	bool initialize(IWidgetFactory* widgetFactory, const StyleSheet* styleSheet);

	/*! Cleanup UI application. */
	void finalize();

	/*! Process system events.
	 *
	 * Process a single system event.
	 * This method will return as soon as there are no
	 * more system events pending.
	 */
	bool process();

	/*! Execute application.
	 *
	 * Process all system events.
	 * This method will block until the application
	 * terminates.
	 */
	int execute();

	/*! Exit application.
	 *
	 * Should be called from the application when
	 * it wants to be terminated.
	 */
	void exit(int exitCode);

	/*! Get system event loop. */
	IEventLoop* getEventLoop();

	/*! Get system widget factory. */
	IWidgetFactory* getWidgetFactory();

	/*! Get clipboard. */
	Clipboard* getClipboard();

	/*! Set style sheet. */
	void setStyleSheet(const StyleSheet* styleSheet);

	/*! Get style sheet. */
	const StyleSheet* getStyleSheet() const;

	/*! Get properties. */
	PropertyGroup* getProperties();

	/*! \name Virtual key translation. */
	//@{

	/*! Translate from key name to code.
	 *
	 * \note Key name is unlocalized, i.e. it's always in English.
	 */
	VirtualKey translateVirtualKey(const std::wstring& keyName) const;

	/*! Translate from key code to name.
	 *
	 * \note Key name is unlocalized, i.e. it's always in English.
	 */
	std::wstring translateVirtualKey(VirtualKey virtualKey) const;

	//@}

private:
	IEventLoop* m_eventLoop;
	IWidgetFactory* m_widgetFactory;
	Ref< Clipboard > m_clipboard;
	Ref< const StyleSheet > m_styleSheet;
	Ref< PropertyGroup > m_properties;
};

/*! Convenience wrappers. */
//@{

inline int32_t getSystemDPI()
{
	return Application::getInstance()->getWidgetFactory()->getSystemDPI();
}

inline int32_t dpi96(int32_t referenceMeasure)
{
	return (getSystemDPI() * referenceMeasure) / 96;
}

inline int32_t invdpi96(int32_t measure)
{
	return (measure * 96) / getSystemDPI();
}

//@}

}
