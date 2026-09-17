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
#include "Llm/ChatTemplate.h"
#include "Ui/Form.h"

#include <string>

namespace traktor
{

class CommandLine;
class Path;

}

namespace traktor::ui
{

class Button;
class ButtonClickEvent;
class CloseEvent;
class Edit;
class KeyDownEvent;
class Static;
class TimerEvent;
class ToolBar;
class ToolBarButtonClickEvent;

}

namespace traktor::llm
{

class ChatView;
class Generator;
class Model;

/*! Main window of the chat application.
 * \ingroup Llm
 *
 * Generation runs on the generator's own thread. This form polls it from a
 * timer rather than being called back, which keeps every widget touched on
 * the thread that owns it.
 */
class ChatForm : public ui::Form
{
	T_RTTI_CLASS;

public:
	bool create(const CommandLine& commandLine);

	virtual void destroy() override;

private:
	Ref< ui::ToolBar > m_toolBar;
	Ref< ChatView > m_chatView;
	Ref< ui::Edit > m_input;
	Ref< ui::Button > m_send;
	Ref< ui::Static > m_status;
	Ref< Model > m_model;
	Ref< Generator > m_generator;

	AlignedVector< ChatMessage > m_conversation;
	std::wstring m_reply;
	int32_t m_replyIndex = -1;
	bool m_generating = false;
	std::wstring m_modelDescription;

	/*! Context length asked for on the command line, and whether it was. */
	int32_t m_contextLength = 0;
	bool m_contextRequested = false;

	bool loadModel(const Path& fileName);

	void openModel();

	void newConversation();

	void submit();

	void finishReply(bool cancelled);

	void updateStatus();

	void updateEnable();

	void notice(const std::wstring& text);

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventInputKeyDown(ui::KeyDownEvent* event);

	void eventSendClick(ui::ButtonClickEvent* event);

	void eventTimer(ui::TimerEvent* event);

	void eventClose(ui::CloseEvent* event);
};

}
