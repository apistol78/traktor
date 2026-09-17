/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/Vocabulary.h"

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Llm/GgufFile.h"

#include <algorithm>

namespace traktor::llm
{
namespace
{

const std::string c_emptyToken;

/*! Byte fallback token text used by SentencePiece vocabularies, e.g. "<0x0A>". */
std::string formatByteToken(uint8_t value)
{
	const char* const hex = "0123456789ABCDEF";
	std::string text = "<0x";
	text += hex[value >> 4];
	text += hex[value & 0x0f];
	text += '>';
	return text;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.llm.Vocabulary", Vocabulary, Object)

bool Vocabulary::create(const GgufFile& file)
{
	for (int32_t i = 0; i < 256; ++i)
		m_byteTokens[i] = -1;

	const std::string model = file.getString(L"tokenizer.ggml.model", "");
	if (model == "llama")
		m_model = TokenizerModel::SentencePiece;
	else if (model == "gpt2")
		m_model = TokenizerModel::BytePair;
	else
	{
		log::error << L"Unsupported tokenizer model \"" << mbstows(model) << L"\"; expected \"llama\" or \"gpt2\"." << Endl;
		return false;
	}

	const GgufValue* tokens = file.findValue(L"tokenizer.ggml.tokens");
	if (tokens == nullptr || !tokens->isArray() || tokens->strings.empty())
	{
		log::error << L"Model has no token table." << Endl;
		return false;
	}

	m_tokens = tokens->strings;
	const size_t tokenCount = m_tokens.size();

	// Scores drive SentencePiece merges; byte pair vocabularies omit them.
	m_scores.resize(tokenCount, 0.0f);
	const GgufValue* scores = file.findValue(L"tokenizer.ggml.scores");
	if (scores != nullptr && scores->isArray())
	{
		const size_t n = std::min(tokenCount, scores->numbers.size());
		for (size_t i = 0; i < n; ++i)
			m_scores[i] = (float)scores->numbers[i];
	}

	m_types.resize(tokenCount, TokenType::Normal);
	const GgufValue* types = file.findValue(L"tokenizer.ggml.token_type");
	if (types != nullptr && types->isArray())
	{
		const size_t n = std::min(tokenCount, types->numbers.size());
		for (size_t i = 0; i < n; ++i)
			m_types[i] = (TokenType)(int32_t)types->numbers[i];
	}

	for (size_t i = 0; i < tokenCount; ++i)
	{
		m_tokenIndex.insert(m_tokens[i], (int32_t)i);

		const TokenType type = m_types[i];
		if (type == TokenType::Control || type == TokenType::UserDefined)
			m_specialTokens.push_back((int32_t)i);
	}

	// Longest first, so that scanning a prompt for markup cannot match a
	// prefix of a longer special token.
	std::sort(m_specialTokens.begin(), m_specialTokens.end(), [&](int32_t a, int32_t b) {
		return m_tokens[a].size() > m_tokens[b].size();
	});

	const GgufValue* merges = file.findValue(L"tokenizer.ggml.merges");
	if (merges != nullptr && merges->isArray())
	{
		for (size_t i = 0; i < merges->strings.size(); ++i)
			m_mergeIndex.insert(merges->strings[i], (int32_t)i);
	}
	else if (m_model == TokenizerModel::BytePair)
	{
		log::error << L"Byte pair vocabulary has no merge table." << Endl;
		return false;
	}

	if (m_model == TokenizerModel::SentencePiece)
	{
		for (int32_t i = 0; i < 256; ++i)
			m_byteTokens[i] = findToken(formatByteToken((uint8_t)i));
	}

	m_beginOfSequence = (int32_t)file.getInteger(L"tokenizer.ggml.bos_token_id", -1);
	m_endOfSequence = (int32_t)file.getInteger(L"tokenizer.ggml.eos_token_id", -1);
	m_unknown = (int32_t)file.getInteger(L"tokenizer.ggml.unknown_token_id", -1);
	m_padding = (int32_t)file.getInteger(L"tokenizer.ggml.padding_token_id", -1);

	m_addBeginOfSequence = file.getBool(L"tokenizer.ggml.add_bos_token", m_model == TokenizerModel::SentencePiece);
	m_addEndOfSequence = file.getBool(L"tokenizer.ggml.add_eos_token", false);

	if (m_endOfSequence >= 0)
		m_endOfGeneration.insert(m_endOfSequence);

	addEndOfGeneration(file, L"tokenizer.ggml.eot_token_id");
	addEndOfGeneration(file, L"tokenizer.ggml.eom_token_id");

	// Chat tuned models end a turn on a marker that is usually not the same
	// identifier as the end of sequence token; without these the model runs
	// straight on into a reply it invents for the user.
	static const char* const c_endOfTurnTokens[] = {
		"<|im_end|>",
		"<|eot_id|>",
		"<|end_of_text|>",
		"<|end|>",
		"<end_of_turn>",
		"</s>"
	};
	for (const char* text : c_endOfTurnTokens)
	{
		const int32_t token = findToken(text);
		if (token >= 0)
			m_endOfGeneration.insert(token);
	}

	log::info << L"Vocabulary: " << (uint32_t)tokenCount << L" tokens, " << (m_model == TokenizerModel::SentencePiece ? L"SentencePiece" : L"byte pair") << L", " << (uint32_t)m_specialTokens.size() << L" special." << Endl;
	return true;
}

const std::string& Vocabulary::getTokenText(int32_t token) const
{
	if (token < 0 || token >= (int32_t)m_tokens.size())
		return c_emptyToken;
	return m_tokens[token];
}

float Vocabulary::getTokenScore(int32_t token) const
{
	if (token < 0 || token >= (int32_t)m_scores.size())
		return 0.0f;
	return m_scores[token];
}

TokenType Vocabulary::getTokenType(int32_t token) const
{
	if (token < 0 || token >= (int32_t)m_types.size())
		return TokenType::Undefined;
	return m_types[token];
}

int32_t Vocabulary::findToken(const std::string& text) const
{
	const int32_t* token = m_tokenIndex.find(text);
	return token != nullptr ? *token : -1;
}

int32_t Vocabulary::findMergeRank(const std::string& left, const std::string& right) const
{
	const int32_t* rank = m_mergeIndex.find(left + " " + right);
	return rank != nullptr ? *rank : -1;
}

bool Vocabulary::isEndOfGeneration(int32_t token) const
{
	return m_endOfGeneration.find(token) != m_endOfGeneration.end();
}

void Vocabulary::addEndOfGeneration(const GgufFile& file, const wchar_t* key)
{
	const int32_t token = (int32_t)file.getInteger(key, -1);
	if (token >= 0)
		m_endOfGeneration.insert(token);
}

}
