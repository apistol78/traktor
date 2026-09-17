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
#include "Core/Ref.h"

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

/*! Which pre-tokenizer splits text before byte pair merging.
 *
 * Byte pair vocabularies are only reproducible if text is first cut at the
 * same places the model was trained with; the pattern is named by
 * "tokenizer.ggml.pre" in the GGUF header.
 */
enum class PreTokenizer
{
	Gpt2,	//!< The original pattern; digit runs of any length.
	Llama3, //!< Adds line break handling; digits group in threes.
	Qwen2	//!< As Llama 3, but one digit per token.
};

/*! Converts between text and token identifiers.
 * \ingroup Llm
 *
 * Operates entirely on bytes. Chat markup that is present in the vocabulary
 * as a control or user defined token is lifted out before merging, so
 * template markup always yields the single token the model expects.
 */
class T_DLLCLASS Tokenizer : public Object
{
	T_RTTI_CLASS;

public:
	bool create(const GgufFile& file, const Vocabulary* vocabulary);

	/*! Tokenize \a text, appending to \a outTokens.
	 *
	 * \param addBeginOfSequence Prepend the begin of sequence token, if the
	 *                           model declares one and asks for it.
	 * \param parseSpecial Recognize control and user defined token text.
	 *                     Enable for prompts built from a chat template,
	 *                     disable for untrusted text that must not be able
	 *                     to inject markup.
	 */
	void encode(const std::string& text, bool addBeginOfSequence, bool parseSpecial, AlignedVector< int32_t >& outTokens) const;

	/*! Text of a single token, as raw bytes.
	 *
	 * The result may end mid sequence for a token that carries only part of
	 * a multi byte character; callers streaming output must buffer until the
	 * bytes form a complete character.
	 */
	std::string decode(int32_t token) const;

	std::string decode(const AlignedVector< int32_t >& tokens) const;

private:
	Ref< const Vocabulary > m_vocabulary;
	PreTokenizer m_preTokenizer = PreTokenizer::Gpt2;
	bool m_addSpacePrefix = true;
	uint32_t m_byteToCodePoint[256];
	AlignedVector< int32_t > m_codePointToByte;

	void encodeRaw(const std::string& text, AlignedVector< int32_t >& outTokens) const;

	void encodeSentencePiece(const std::string& text, AlignedVector< int32_t >& outTokens) const;

	void encodeBytePair(const std::string& text, AlignedVector< int32_t >& outTokens) const;

	void emitUnknown(const std::string& text, AlignedVector< int32_t >& outTokens) const;

	std::string encodeByteLevel(const std::string& text) const;

	std::string decodeByteLevel(const std::string& text) const;
};

}
