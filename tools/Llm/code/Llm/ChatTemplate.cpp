/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/ChatTemplate.h"

#include "Core/Log/Log.h"
#include "Llm/GgufFile.h"
#include "Llm/Vocabulary.h"

namespace traktor::llm
{
namespace
{

bool contains(const std::string& text, const char* needle)
{
	return text.find(needle) != std::string::npos;
}

const char* getRoleName(ChatRole role)
{
	switch (role)
	{
	case ChatRole::System:
		return "system";
	case ChatRole::Assistant:
		return "assistant";
	default:
		return "user";
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.llm.ChatTemplate", ChatTemplate, Object)

bool ChatTemplate::create(const GgufFile& file, const Vocabulary* vocabulary)
{
	const std::string jinja = file.getString(L"tokenizer.chat_template", "");

	// Recognize the family from the markers the template mentions rather than
	// from the model name, which is frequently wrong in redistributed files.
	if (contains(jinja, "<|im_start|>"))
		m_kind = ChatTemplateKind::ChatMl;
	else if (contains(jinja, "<|start_header_id|>"))
		m_kind = ChatTemplateKind::Llama3;
	else if (contains(jinja, "[INST]"))
		m_kind = ChatTemplateKind::Llama2;
	else if (vocabulary != nullptr)
	{
		// No usable template; fall back on whichever markup the vocabulary
		// actually has tokens for.
		if (vocabulary->findToken("<|im_start|>") >= 0)
			m_kind = ChatTemplateKind::ChatMl;
		else if (vocabulary->findToken("<|start_header_id|>") >= 0)
			m_kind = ChatTemplateKind::Llama3;
		else if (vocabulary->findToken("[INST]") >= 0 || file.getArchitecture() == L"llama")
			m_kind = ChatTemplateKind::Llama2;
		else
			m_kind = ChatTemplateKind::Plain;
	}

	if (m_kind == ChatTemplateKind::Plain)
		log::warning << L"No recognized chat template; the model will complete text rather than answer." << Endl;
	else
		log::info << L"Chat template: " << getKindName() << L"." << Endl;

	return true;
}

std::wstring ChatTemplate::getKindName() const
{
	switch (m_kind)
	{
	case ChatTemplateKind::ChatMl:
		return L"ChatML";
	case ChatTemplateKind::Llama3:
		return L"Llama 3";
	case ChatTemplateKind::Llama2:
		return L"Llama 2";
	default:
		return L"plain";
	}
}

bool ChatTemplate::getSupportsSystemRole() const
{
	return m_kind == ChatTemplateKind::ChatMl || m_kind == ChatTemplateKind::Llama3 || m_kind == ChatTemplateKind::Llama2;
}

std::string ChatTemplate::format(const AlignedVector< ChatMessage >& messages) const
{
	std::string prompt;

	switch (m_kind)
	{
	case ChatTemplateKind::ChatMl:
		{
			for (const auto& message : messages)
			{
				prompt += "<|im_start|>";
				prompt += getRoleName(message.role);
				prompt += "\n";
				prompt += message.content;
				prompt += "<|im_end|>\n";
			}
			prompt += "<|im_start|>assistant\n";
		}
		break;

	case ChatTemplateKind::Llama3:
		{
			// The begin of text token is the vocabulary's own, and is added by
			// the tokenizer; emitting it here would double it.
			for (const auto& message : messages)
			{
				prompt += "<|start_header_id|>";
				prompt += getRoleName(message.role);
				prompt += "<|end_header_id|>\n\n";
				prompt += message.content;
				prompt += "<|eot_id|>";
			}
			prompt += "<|start_header_id|>assistant<|end_header_id|>\n\n";
		}
		break;

	case ChatTemplateKind::Llama2:
		{
			// Llama 2 has no turn of its own for the system prompt; it rides
			// inside the first instruction block.
			std::string systemPrompt;
			for (const auto& message : messages)
			{
				if (message.role == ChatRole::System)
					systemPrompt = message.content;
			}

			bool first = true;
			for (const auto& message : messages)
			{
				if (message.role == ChatRole::System)
					continue;

				if (message.role == ChatRole::User)
				{
					// Every instruction block after the first opens a new
					// sequence; the tokenizer supplies the first one.
					if (!first)
						prompt += "<s>";

					prompt += "[INST] ";
					if (first && !systemPrompt.empty())
					{
						prompt += "<<SYS>>\n";
						prompt += systemPrompt;
						prompt += "\n<</SYS>>\n\n";
					}
					prompt += message.content;
					prompt += " [/INST]";
					first = false;
				}
				else
				{
					prompt += " ";
					prompt += message.content;
					prompt += "</s>";
				}
			}

			if (first)
			{
				// Conversation opened with an assistant turn; nothing to answer.
				prompt += "[INST]  [/INST]";
			}
		}
		break;

	default:
		{
			for (const auto& message : messages)
			{
				if (message.role == ChatRole::System)
					continue;
				prompt += message.content;
				prompt += "\n";
			}
		}
		break;
	}

	return prompt;
}

}
