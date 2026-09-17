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
#include "Core/Containers/SmallSet.h"
#include "Core/Containers/StringMap.h"
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

/*! Classification of a vocabulary entry, as stored in GGUF. */
enum class TokenType : int32_t
{
	Undefined = 0,
	Normal = 1,
	Unknown = 2,
	Control = 3,
	UserDefined = 4,
	Unused = 5,
	Byte = 6
};

/*! Which tokenization algorithm the vocabulary was built for. */
enum class TokenizerModel
{
	Unknown,
	SentencePiece, //!< "llama" in GGUF; score driven merges with <0xNN> byte fallback.
	BytePair	   //!< "gpt2" in GGUF; rank driven merges over byte level text.
};

/*! Token table of a model, read from the GGUF header.
 * \ingroup Llm
 *
 * Token text is kept as raw bytes throughout. For SentencePiece that is
 * UTF-8 with U+2581 standing in for a space; for byte pair vocabularies it
 * is the reversible byte level encoding, which is only valid UTF-8 by
 * coincidence. Either way it must not be widened until it has been decoded.
 */
class T_DLLCLASS Vocabulary : public Object
{
	T_RTTI_CLASS;

public:
	bool create(const GgufFile& file);

	TokenizerModel getModel() const { return m_model; }

	int32_t getTokenCount() const { return (int32_t)m_tokens.size(); }

	const std::string& getTokenText(int32_t token) const;

	float getTokenScore(int32_t token) const;

	TokenType getTokenType(int32_t token) const;

	/*! Identifier of \a text, or -1 if it is not in the vocabulary. */
	int32_t findToken(const std::string& text) const;

	/*! Merge priority of the pair \a left, \a right, or -1 if it does not merge.
	 *
	 * Lower ranks merge first. Only meaningful for byte pair vocabularies.
	 */
	int32_t findMergeRank(const std::string& left, const std::string& right) const;

	/*! Identifier encoding the single byte \a value, or -1 if unavailable. */
	int32_t getByteToken(uint8_t value) const { return m_byteTokens[value]; }

	/*! Control and user defined tokens, longest text first.
	 *
	 * Used to carve chat template markup out of a prompt before the regular
	 * tokenizer sees it, so "<|im_start|>" stays one token.
	 */
	const AlignedVector< int32_t >& getSpecialTokens() const { return m_specialTokens; }

	int32_t getBeginOfSequence() const { return m_beginOfSequence; }

	int32_t getEndOfSequence() const { return m_endOfSequence; }

	int32_t getUnknown() const { return m_unknown; }

	int32_t getPadding() const { return m_padding; }

	bool getAddBeginOfSequence() const { return m_addBeginOfSequence; }

	bool getAddEndOfSequence() const { return m_addEndOfSequence; }

	/*! True if \a token should stop generation.
	 *
	 * Covers the end of sequence token plus the end of turn markers used by
	 * chat tuned models, which are frequently not the same identifier.
	 */
	bool isEndOfGeneration(int32_t token) const;

private:
	TokenizerModel m_model = TokenizerModel::Unknown;
	AlignedVector< std::string > m_tokens;
	AlignedVector< float > m_scores;
	AlignedVector< TokenType > m_types;
	AlignedVector< int32_t > m_specialTokens;
	SmallSet< int32_t > m_endOfGeneration;
	StringMap< int32_t, std::string, 16384 > m_tokenIndex;
	StringMap< int32_t, std::string, 16384 > m_mergeIndex;
	int32_t m_byteTokens[256];
	int32_t m_beginOfSequence = -1;
	int32_t m_endOfSequence = -1;
	int32_t m_unknown = -1;
	int32_t m_padding = -1;
	bool m_addBeginOfSequence = true;
	bool m_addEndOfSequence = false;

	void addEndOfGeneration(const GgufFile& file, const wchar_t* key);
};

}
