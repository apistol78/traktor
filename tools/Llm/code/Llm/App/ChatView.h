/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Ref.h"
#include "Ui/Widget.h"

#include <string>

namespace traktor::ui
{

class ScrollBar;
class StyleSheet;

}

namespace traktor::llm
{

/*! Scrolling transcript of a conversation.
 * \ingroup Llm
 *
 * Lays out and draws its own bubbles rather than using a text control,
 * because none of them wrap; wrapping is the whole job here.
 */
class ChatView : public ui::Widget
{
	T_RTTI_CLASS;

public:
	enum class Speaker
	{
		User,
		Assistant,
		Notice //!< Engine messages: errors, cancellation, context warnings.
	};

	bool create(ui::Widget* parent);

	/*! Append a message, returning its index. */
	int32_t addMessage(Speaker speaker, const std::wstring& text);

	/*! Replace the text of a message already shown. */
	void setMessageText(int32_t index, const std::wstring& text);

	void removeAllMessages();

	int32_t getMessageCount() const { return (int32_t)m_messages.size(); }

	/*! Colors used by the transcript, merged over the default sheet. */
	static Ref< ui::StyleSheet > createStyleSheet();

private:
	class Message
	{
	public:
		Speaker speaker = Speaker::User;
		std::wstring text;
		AlignedVector< std::wstring > lines;
		int32_t top = 0;
		int32_t height = 0;
		int32_t width = 0;		 //!< Width of the widest line, for the bubble.
		int32_t layoutWidth = -1; //!< Wrap width the lines were built for, -1 when stale.
	};

	Ref< ui::ScrollBar > m_scrollBar;
	AlignedVector< Message > m_messages;
	int32_t m_contentHeight = 0;
	bool m_followTail = true;

	void layout();

	void updateScrollBar();

	void scrollToTail();

	void eventPaint(ui::PaintEvent* event);

	void eventSize(ui::SizeEvent* event);

	void eventScroll(ui::ScrollEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);
};

}
