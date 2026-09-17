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
#include "Core/Object.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_LLM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::llm
{

class GgufFile;
class Vocabulary;

enum class ChatRole
{
	System,
	User,
	Assistant
};

/*! One turn of a conversation. */
class T_DLLCLASS ChatMessage
{
public:
	ChatRole role = ChatRole::User;
	std::string content;

	ChatMessage() = default;

	ChatMessage(ChatRole role_, const std::string& content_)
		: role(role_)
		, content(content_)
	{
	}
};

/*! Markup family a model was instruction tuned with. */
enum class ChatTemplateKind
{
	Plain,	//!< No markup; the prompt is fed verbatim.
	ChatMl, //!< <|im_start|>role ... <|im_end|>, used by Qwen among others.
	Llama3, //!< <|start_header_id|>role<|end_header_id|> ... <|eot_id|>
	Llama2	//!< [INST] ... [/INST], used by Llama 2 and Mistral.
};

/*! Wraps a conversation in the markup a chat tuned model expects.
 * \ingroup Llm
 *
 * GGUF carries the real template as a Jinja program, which is far more than
 * this engine needs to interpret. The family is recognized from the markers
 * the template mentions, and a hand written formatter is used for it; an
 * unrecognized template degrades to feeding the text through unchanged,
 * which produces a completion rather than a reply.
 */
class T_DLLCLASS ChatTemplate : public Object
{
	T_RTTI_CLASS;

public:
	bool create(const GgufFile& file, const Vocabulary* vocabulary);

	ChatTemplateKind getKind() const { return m_kind; }

	std::wstring getKindName() const;

	/*! True if the family has a dedicated system turn.
	 *
	 * Where it has not, a system prompt is folded into the first user turn.
	 */
	bool getSupportsSystemRole() const;

	/*! Render \a messages, ending with the marker that opens a reply. */
	std::string format(const AlignedVector< ChatMessage >& messages) const;

private:
	ChatTemplateKind m_kind = ChatTemplateKind::Plain;
};

}
