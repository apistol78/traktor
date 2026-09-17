/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/App/ChatForm.h"

#include "Core/Io/Path.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"
#include "Llm/App/ChatView.h"
#include "Llm/Utf8.h"
#include "Llm/Context.h"
#include "Llm/Generator.h"
#include "Llm/Model.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/Events/ButtonClickEvent.h"
#include "Ui/Events/CloseEvent.h"
#include "Ui/Events/KeyDownEvent.h"
#include "Ui/Events/TimerEvent.h"
#include "Ui/FileDialog.h"
#include "Ui/MessageBox.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

namespace traktor::llm
{
namespace
{

const int32_t c_pollInterval = 50;
const int32_t c_maximumReplyTokens = 1024;


const wchar_t* const c_systemPrompt = L"You are a helpful assistant. Answer concisely.";

/*! Model text is UTF-8 regardless of the platform locale, so say so. */
std::wstring fromUtf8(const std::string& text)
{
	return widenUtf8(text);
}

std::string toUtf8(const std::wstring& text)
{
	return wstombs(Utf8Encoding(), text);
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.llm.ChatForm", ChatForm, ui::Form)

bool ChatForm::create(const CommandLine& commandLine)
{
	if (!ui::Form::create(
			L"Traktor Chat",
			900_ut,
			700_ut,
			ui::Form::WsDefault,
			new ui::TableLayout(L"100%", L"*,100%,*,*", 0_ut, 0_ut)))
		return false;

	addEventHandler< ui::CloseEvent >(this, &ChatForm::eventClose);
	addEventHandler< ui::TimerEvent >(this, &ChatForm::eventTimer);

	m_toolBar = new ui::ToolBar();
	if (!m_toolBar->create(this))
		return false;

	m_toolBar->addItem(new ui::ToolBarButton(L"Open model...", ui::Command(L"Llm.OpenModel")));
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(new ui::ToolBarButton(L"New chat", ui::Command(L"Llm.NewChat")));
	m_toolBar->addItem(new ui::ToolBarButton(L"Stop", ui::Command(L"Llm.Stop")));
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ChatForm::eventToolBarClick);

	m_chatView = new ChatView();
	if (!m_chatView->create(this))
		return false;

	Ref< ui::Container > inputRow = new ui::Container();
	if (!inputRow->create(this, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 8_ut, 8_ut)))
		return false;

	m_input = new ui::Edit();
	if (!m_input->create(inputRow, L""))
		return false;
	m_input->addEventHandler< ui::KeyDownEvent >(this, &ChatForm::eventInputKeyDown);

	m_send = new ui::Button();
	if (!m_send->create(inputRow, L"Send"))
		return false;
	m_send->addEventHandler< ui::ButtonClickEvent >(this, &ChatForm::eventSendClick);

	m_status = new ui::Static();
	if (!m_status->create(this, L"No model loaded."))
		return false;

	m_contextRequested = commandLine.hasOption(L"context");
	m_contextLength = m_contextRequested ? commandLine.getOption(L"context").getInteger() : c_defaultContextLength;

	if (m_contextLength <= 0)
	{
		log::warning << L"Ignoring a context of " << m_contextLength << L"; using " << c_defaultContextLength << L"." << Endl;
		m_contextLength = c_defaultContextLength;
		m_contextRequested = false;
	}

	newConversation();
	updateEnable();

	update();
	show();

	startTimer(c_pollInterval);

	// A model named on the command line loads straight away.
	if (commandLine.getCount() > 0)
	{
		if (!loadModel(Path(commandLine.getString(0))))
			notice(L"Unable to load " + commandLine.getString(0) + L".");
	}
	else
		notice(L"Open a GGUF model to begin.");

	return true;
}

void ChatForm::destroy()
{
	stopTimer();

	safeDestroy(m_generator);
	m_model = nullptr;

	ui::Form::destroy();
}

bool ChatForm::loadModel(const Path& fileName)
{
	// Tear down whatever is loaded first; two models of any size will not
	// both fit comfortably.
	safeDestroy(m_generator);
	m_model = nullptr;

	Ref< Model > model = new Model();
	if (!model->create(fileName))
		return false;

	// Only complain when the shortfall is something the user asked for; the
	// default landing above a small model's trained length is unremarkable.
	const int32_t trained = model->getParameters().contextLength;
	int32_t contextLength = m_contextLength;

	if (trained > 0 && contextLength > trained)
	{
		if (m_contextRequested)
			log::warning << L"Requested context of " << contextLength << L" exceeds the " << trained << L" this model was trained for; using " << trained << L"." << Endl;
		contextLength = trained;
	}

	Ref< Generator > generator = new Generator();
	if (!generator->create(model, contextLength))
		return false;

	m_model = model;
	m_generator = generator;
	m_modelDescription = model->getDescription();

	newConversation();
	updateEnable();
	updateStatus();

	notice(L"Loaded " + m_modelDescription + L".");

	// The context in use is rarely the one the model advertises, and the gap
	// is worth seeing without reading the log.
	if (trained > 0 && contextLength < trained)
		notice(L"Using " + toString(contextLength) + L" of " + toString(trained) + L" context positions; pass -context=N to change.");

	return true;
}

void ChatForm::openModel()
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(this, type_name(this), L"Open model", L"GGUF model;*.gguf;All files;*.*"))
		return;

	Path fileName;
	if (fileDialog.showModalThenDestroy(fileName) != ui::DialogResult::Ok)
		return;

	if (!loadModel(fileName))
		ui::MessageBox::show(this, L"Unable to load model; see the log for details.", L"Traktor Chat", ui::MbIconError | ui::MbOk);
}

void ChatForm::newConversation()
{
	if (m_generating)
		return;

	m_conversation.clear();
	m_conversation.push_back(ChatMessage(ChatRole::System, toUtf8(c_systemPrompt)));

	m_reply.clear();
	m_replyIndex = -1;

	if (m_chatView)
		m_chatView->removeAllMessages();

	if (m_generator)
		m_generator->resetConversation();

	updateStatus();
}

void ChatForm::submit()
{
	if (m_generating || !m_generator)
		return;

	const std::wstring text = trim(m_input->getText());
	if (text.empty())
		return;

	m_input->setText(L"");
	m_input->update();

	m_chatView->addMessage(ChatView::Speaker::User, text);
	m_conversation.push_back(ChatMessage(ChatRole::User, toUtf8(text)));

	// The reply bubble appears immediately and fills in as tokens arrive.
	m_reply.clear();
	m_replyIndex = m_chatView->addMessage(ChatView::Speaker::Assistant, L"");

	if (!m_generator->begin(m_conversation, c_maximumReplyTokens))
		return;

	m_generating = true;
	updateEnable();
}

void ChatForm::finishReply(bool cancelled)
{
	m_generating = false;

	if (m_replyIndex >= 0)
	{
		if (m_reply.empty())
			m_chatView->setMessageText(m_replyIndex, cancelled ? L"(stopped)" : L"(no reply)");

		// Keep the partial reply in the conversation; the model should see
		// what it already said, exactly as the user did.
		if (!m_reply.empty())
			m_conversation.push_back(ChatMessage(ChatRole::Assistant, toUtf8(m_reply)));
	}

	m_replyIndex = -1;
	updateEnable();
	updateStatus();
}

void ChatForm::updateStatus()
{
	if (!m_generator)
	{
		m_status->setText(L"No model loaded.");
		return;
	}

	int32_t used = 0;
	int32_t available = 0;
	m_generator->getContextUsage(used, available);

	StringOutputStream ss;
	ss << m_modelDescription;
	ss << L"   |   context " << used << L"/" << available;

	const double promptRate = m_generator->getPromptRate();
	const double generateRate = m_generator->getGenerateRate();

	if (promptRate > 0.0)
		ss << L"   |   prompt " << toString(promptRate, 1) << L" tok/s";
	if (generateRate > 0.0)
		ss << L"   |   reply " << toString(generateRate, 1) << L" tok/s";

	m_status->setText(ss.str());
	m_status->update();
}

void ChatForm::updateEnable()
{
	const bool ready = (m_generator != nullptr);

	m_input->setEnable(ready && !m_generating);
	m_send->setEnable(ready && !m_generating);

	m_toolBar->update();
}

void ChatForm::notice(const std::wstring& text)
{
	if (m_chatView)
		m_chatView->addMessage(ChatView::Speaker::Notice, text);
}

void ChatForm::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();

	if (command == L"Llm.OpenModel")
		openModel();
	else if (command == L"Llm.NewChat")
		newConversation();
	else if (command == L"Llm.Stop")
	{
		if (m_generator)
			m_generator->cancel();
	}
}

void ChatForm::eventInputKeyDown(ui::KeyDownEvent* event)
{
	if (event->getVirtualKey() == ui::VkReturn)
	{
		submit();
		event->consume();
	}
}

void ChatForm::eventSendClick(ui::ButtonClickEvent* event)
{
	submit();
}

void ChatForm::eventTimer(ui::TimerEvent* event)
{
	if (!m_generator)
		return;

	// Drain whatever the generator has produced since the last tick. Doing
	// it from a timer keeps every widget call on the thread that owns it.
	const std::string produced = m_generator->flushText();
	if (!produced.empty() && m_replyIndex >= 0)
	{
		m_reply += fromUtf8(produced);
		m_chatView->setMessageText(m_replyIndex, m_reply);
	}

	if (m_generating)
	{
		updateStatus();

		const GeneratorState state = m_generator->getState();
		if (state != GeneratorState::Prompt && state != GeneratorState::Generating)
		{
			finishReply(state == GeneratorState::Cancelled);

			const std::wstring message = m_generator->getMessage();
			if (!message.empty())
				notice(message);
		}
	}
}

void ChatForm::eventClose(ui::CloseEvent* event)
{
	if (m_generator)
		m_generator->cancel();

	ui::Application::getInstance()->exit(0);
}

}
