/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/Generator.h"

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Llm/Context.h"
#include "Llm/Model.h"
#include "Llm/Tokenizer.h"
#include "Llm/Utf8.h"
#include "Llm/Vocabulary.h"

#include <algorithm>

namespace traktor::llm
{
namespace
{

// Never let the conversation crowd the reply out of more than this share of
// the context, however many tokens the caller asked for.
const int32_t c_maximumReserveShare = 2;


}

T_IMPLEMENT_RTTI_CLASS(L"traktor.llm.Generator", Generator, Object)

Generator::~Generator()
{
	destroy();
}

bool Generator::create(const Model* model, int32_t contextLength)
{
	if (model == nullptr)
		return false;

	m_model = model;

	m_context = new Context();
	if (!m_context->create(model, contextLength))
		return false;

	m_sampler = new Sampler();

	m_thread = ThreadManager::getInstance().create([this]() {
		threadGenerate();
	},
		L"LLM generator");

	if (m_thread == nullptr || !m_thread->start())
	{
		log::error << L"Unable to start generator thread." << Endl;
		return false;
	}

	return true;
}

void Generator::destroy()
{
	cancel();

	if (m_thread != nullptr)
	{
		m_thread->stop();
		ThreadManager::getInstance().destroy(m_thread);
		m_thread = nullptr;
	}

	safeDestroy(m_context);
	m_sampler = nullptr;
	m_model = nullptr;
}

bool Generator::begin(const AlignedVector< ChatMessage >& messages, int32_t maximumTokens)
{
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		if (m_state == GeneratorState::Prompt || m_state == GeneratorState::Generating)
			return false;

		m_messages = messages;
		m_maximumTokens = (maximumTokens > 0) ? maximumTokens : m_context->getContextLength();
		m_state = GeneratorState::Prompt;
		m_message.clear();
		m_text.clear();
		m_partial.clear();
		m_promptTokenCount = 0;
		m_generatedTokenCount = 0;
		m_promptRate = 0.0;
		m_generateRate = 0.0;
		m_cancel = false;
	}

	m_request.set();
	return true;
}

void Generator::cancel()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_cancel = true;
}

GeneratorState Generator::getState() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_state;
}

bool Generator::isBusy() const
{
	const GeneratorState state = getState();
	return state == GeneratorState::Prompt || state == GeneratorState::Generating;
}

std::string Generator::flushText()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	std::string text;
	text.swap(m_text);
	return text;
}

std::wstring Generator::getMessage() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_message;
}

int32_t Generator::getPromptTokenCount() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_promptTokenCount;
}

int32_t Generator::getGeneratedTokenCount() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_generatedTokenCount;
}

double Generator::getPromptRate() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_promptRate;
}

double Generator::getGenerateRate() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_generateRate;
}

void Generator::getContextUsage(int32_t& outUsed, int32_t& outAvailable) const
{
	outUsed = m_context ? m_context->getPosition() : 0;
	outAvailable = m_context ? m_context->getContextLength() : 0;
}

void Generator::setSamplerSettings(const SamplerSettings& settings)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_sampler)
		m_sampler->setSettings(settings);
}

SamplerSettings Generator::getSamplerSettings() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	return m_sampler ? m_sampler->getSettings() : SamplerSettings();
}

void Generator::resetConversation()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// The worker owns the evaluated token list while it runs, so refuse
	// rather than clear it out from under it.
	if (m_state == GeneratorState::Prompt || m_state == GeneratorState::Generating)
		return;

	m_evaluated.clear();
	if (m_context)
		m_context->reset();
}

void Generator::threadGenerate()
{
	while (!m_thread->stopped())
	{
		if (!m_request.wait(100))
			continue;

		m_request.reset();

		if (m_thread->stopped())
			break;

		generate();
	}
}

void Generator::generate()
{
	AlignedVector< ChatMessage > messages;
	int32_t maximumTokens;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		messages = m_messages;
		maximumTokens = m_maximumTokens;
	}

	AlignedVector< int32_t > tokens;
	if (!buildPrompt(messages, maximumTokens, tokens))
	{
		setState(GeneratorState::Failed, L"Conversation does not fit in the context.");
		return;
	}

	if (tokens.empty())
	{
		setState(GeneratorState::Failed, L"Prompt is empty.");
		return;
	}

	const Tokenizer* tokenizer = m_model->getTokenizer();
	const Vocabulary* vocabulary = m_model->getVocabulary();

	// Reuse whatever prefix of the cache the new prompt still agrees with.
	// Every turn repeats the whole conversation, so this is usually all but
	// the last exchange.
	size_t shared = 0;
	while (shared < m_evaluated.size() && shared + 1 < tokens.size() && m_evaluated[shared] == tokens[shared])
		++shared;

	m_context->rewind((int32_t)shared);
	m_evaluated.resize(shared);

	Timer timer;
	timer.reset();

	setState(GeneratorState::Prompt, L"");

	for (size_t i = shared; i < tokens.size(); ++i)
	{
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			if (m_cancel)
			{
				setState(GeneratorState::Cancelled, L"");
				return;
			}
			m_promptTokenCount = (int32_t)(i - shared + 1);
		}

		// Only the final prompt token needs logits; the rest are read purely
		// to fill the cache, and skipping the output projection for them is
		// the cheapest speedup available here.
		const bool last = (i + 1 == tokens.size());
		if (!m_context->evaluate(tokens[i], last))
		{
			setState(GeneratorState::Failed, L"Context is full.");
			return;
		}

		m_evaluated.push_back(tokens[i]);
	}

	{
		const double elapsed = timer.getElapsedTime();
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (elapsed > 0.0 && m_promptTokenCount > 0)
			m_promptRate = m_promptTokenCount / elapsed;
	}

	setState(GeneratorState::Generating, L"");

	timer.reset();
	int32_t generated = 0;
	bool emitted = false;

	while (generated < maximumTokens)
	{
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			if (m_cancel)
			{
				setState(GeneratorState::Cancelled, L"");
				return;
			}
		}

		float* logits = m_context->getLogits();
		if (logits == nullptr)
		{
			setState(GeneratorState::Failed, L"No logits produced.");
			return;
		}

		int32_t token;
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			token = m_sampler->sample(logits, m_context->getLogitCount(), m_evaluated);
		}

		if (token < 0 || vocabulary->isEndOfGeneration(token))
			break;

		{
			std::string piece = tokenizer->decode(token);

			// A reply opens on a word boundary, so the space the first token
			// carries is markup rather than content.
			if (!emitted)
			{
				const size_t start = piece.find_first_not_of(" \t\n\r");
				piece = (start == std::string::npos) ? std::string() : piece.substr(start);
			}

			if (!piece.empty())
				emitted = true;

			appendText(piece);
		}

		++generated;
		{
			const double elapsed = timer.getElapsedTime();
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			m_generatedTokenCount = generated;
			if (elapsed > 0.0)
				m_generateRate = generated / elapsed;
		}

		m_evaluated.push_back(token);

		if (!m_context->evaluate(token, true))
		{
			setState(GeneratorState::Finished, L"Context is full; the reply was cut short.");
			return;
		}
	}

	// Anything still held back was an incomplete character that never
	// completed; emit it rather than losing it.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_text += m_partial;
		m_partial.clear();
	}

	setState(GeneratorState::Finished, L"");
}

bool Generator::buildPrompt(const AlignedVector< ChatMessage >& messages, int32_t maximumTokens, AlignedVector< int32_t >& outTokens)
{
	const Tokenizer* tokenizer = m_model->getTokenizer();
	const ChatTemplate* chatTemplate = m_model->getChatTemplate();

	// Reserve room for the reply the caller asked for, but never more than
	// half the context; a short context would otherwise leave nothing for the
	// conversation itself.
	const int32_t contextLength = m_context->getContextLength();
	const int32_t reserve = std::max(1, std::min(maximumTokens, contextLength / c_maximumReserveShare));
	const int32_t limit = std::max(1, contextLength - reserve);

	AlignedVector< ChatMessage > kept = messages;

	for (;;)
	{
		outTokens.resize(0);
		tokenizer->encode(chatTemplate->format(kept), true, true, outTokens);

		if ((int32_t)outTokens.size() <= limit)
			return true;

		// Drop the oldest turn that is not the system prompt, and try again.
		size_t drop = kept.size();
		for (size_t i = 0; i < kept.size(); ++i)
		{
			if (kept[i].role != ChatRole::System)
			{
				drop = i;
				break;
			}
		}

		// Nothing left to drop but the turn being answered.
		if (drop >= kept.size() || kept.size() <= 1)
			return false;

		kept.erase(kept.begin() + drop);
	}
}

void Generator::appendText(const std::string& text)
{
	if (text.empty())
		return;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_partial += text;

	const size_t complete = getValidUtf8Length(m_partial);
	if (complete > 0)
	{
		m_text += m_partial.substr(0, complete);
		m_partial.erase(0, complete);
	}
}

void Generator::setState(GeneratorState state, const std::wstring& message)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_state = state;
	if (!message.empty())
		m_message = message;
}

}
