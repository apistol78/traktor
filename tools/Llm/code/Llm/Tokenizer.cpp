/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/Tokenizer.h"

#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Llm/GgufFile.h"
#include "Llm/Vocabulary.h"

#include <algorithm>
#include <functional>
#include <queue>

namespace traktor::llm
{
namespace
{

/*! U+2581 LOWER ONE EIGHTH BLOCK, which SentencePiece writes instead of a space. */
const char* const c_spaceMarker = "\xe2\x96\x81";

/*! Value of a single hexadecimal digit, or -1 if \a ch is not one. */
int32_t parseHexDigit(char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	else if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	else
		return -1;
}

/*! Length in bytes of the UTF-8 sequence introduced by \a lead. */
uint32_t getSequenceLength(uint8_t lead)
{
	if ((lead & 0x80) == 0x00)
		return 1;
	else if ((lead & 0xe0) == 0xc0)
		return 2;
	else if ((lead & 0xf0) == 0xe0)
		return 3;
	else if ((lead & 0xf8) == 0xf0)
		return 4;
	else
		return 1; // Stray continuation byte; step over it so we cannot loop.
}

void appendCodePoint(std::string& outText, uint32_t codePoint)
{
	if (codePoint < 0x80)
		outText += (char)codePoint;
	else if (codePoint < 0x800)
	{
		outText += (char)(0xc0 | (codePoint >> 6));
		outText += (char)(0x80 | (codePoint & 0x3f));
	}
	else if (codePoint < 0x10000)
	{
		outText += (char)(0xe0 | (codePoint >> 12));
		outText += (char)(0x80 | ((codePoint >> 6) & 0x3f));
		outText += (char)(0x80 | (codePoint & 0x3f));
	}
	else
	{
		outText += (char)(0xf0 | (codePoint >> 18));
		outText += (char)(0x80 | ((codePoint >> 12) & 0x3f));
		outText += (char)(0x80 | ((codePoint >> 6) & 0x3f));
		outText += (char)(0x80 | (codePoint & 0x3f));
	}
}

/*! Text decoded into code points, retaining the byte offset of each. */
class CodePointView
{
public:
	explicit CodePointView(const std::string& text)
	{
		size_t i = 0;
		while (i < text.size())
		{
			const uint32_t length = getSequenceLength((uint8_t)text[i]);
			const size_t available = std::min((size_t)length, text.size() - i);

			uint32_t codePoint = (uint8_t)text[i];
			if (length == 2 && available == 2)
				codePoint = ((codePoint & 0x1f) << 6) | ((uint8_t)text[i + 1] & 0x3f);
			else if (length == 3 && available == 3)
				codePoint = ((codePoint & 0x0f) << 12) | (((uint8_t)text[i + 1] & 0x3f) << 6) | ((uint8_t)text[i + 2] & 0x3f);
			else if (length == 4 && available == 4)
				codePoint = ((codePoint & 0x07) << 18) | (((uint8_t)text[i + 1] & 0x3f) << 12) | (((uint8_t)text[i + 2] & 0x3f) << 6) | ((uint8_t)text[i + 3] & 0x3f);

			m_codePoints.push_back(codePoint);
			m_offsets.push_back((uint32_t)i);
			i += available;
		}
		m_offsets.push_back((uint32_t)text.size());
	}

	size_t size() const { return m_codePoints.size(); }

	uint32_t operator [] (size_t index) const { return m_codePoints[index]; }

	uint32_t getOffset(size_t index) const { return m_offsets[index]; }

private:
	AlignedVector< uint32_t > m_codePoints;
	AlignedVector< uint32_t > m_offsets;
};

bool isWhitespace(uint32_t codePoint)
{
	switch (codePoint)
	{
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x20:
	case 0x85:
	case 0xa0:
	case 0x1680:
	case 0x2028:
	case 0x2029:
	case 0x202f:
	case 0x205f:
	case 0x3000:
		return true;
	default:
		return codePoint >= 0x2000 && codePoint <= 0x200a;
	}
}

bool isNewLine(uint32_t codePoint)
{
	return codePoint == 0x0a || codePoint == 0x0d;
}

bool isDigit(uint32_t codePoint)
{
	return codePoint >= '0' && codePoint <= '9';
}

/*! Approximate test for the Unicode letter category.
 *
 * Carrying the real category tables would cost more than this engine gains
 * from them, so everything outside ASCII counts as a letter unless it falls
 * in a range that is predominantly punctuation, symbols or emoji. Scripts
 * such as Cyrillic, Greek, Hebrew, Arabic and CJK classify correctly;
 * isolated symbols in an otherwise unlisted block do not, which can shift a
 * token boundary and so change tokenization of text that mixes them.
 */
bool isLetter(uint32_t codePoint)
{
	if (codePoint < 0x80)
		return (codePoint >= 'a' && codePoint <= 'z') || (codePoint >= 'A' && codePoint <= 'Z');

	if (isWhitespace(codePoint))
		return false;

	// Latin-1 punctuation and symbols, plus the two stray math signs.
	if (codePoint <= 0xbf || codePoint == 0xd7 || codePoint == 0xf7)
		return false;

	// General punctuation, currency, symbols, arrows, math, dingbats.
	if (codePoint >= 0x2000 && codePoint <= 0x2bff)
		return false;

	// CJK punctuation.
	if (codePoint >= 0x3000 && codePoint <= 0x303f)
		return false;

	// Emoji and pictographs.
	if (codePoint >= 0x1f000 && codePoint <= 0x1faff)
		return false;

	return true;
}

bool isSymbol(uint32_t codePoint)
{
	return !isWhitespace(codePoint) && !isLetter(codePoint) && !isDigit(codePoint);
}

/*! Match one of the English contraction suffixes at \a i, returning its end. */
int32_t matchContraction(const CodePointView& view, size_t i, bool caseInsensitive)
{
	if (view[i] != '\'')
		return -1;

	static const char* const c_suffixes[] = { "re", "ve", "ll", "s", "t", "m", "d" };
	for (const char* suffix : c_suffixes)
	{
		const size_t length = std::char_traits< char >::length(suffix);
		if (i + 1 + length > view.size())
			continue;

		bool match = true;
		for (size_t j = 0; j < length && match; ++j)
		{
			uint32_t codePoint = view[i + 1 + j];
			if (caseInsensitive && codePoint >= 'A' && codePoint <= 'Z')
				codePoint += 'a' - 'A';
			match = (codePoint == (uint32_t)suffix[j]);
		}

		if (match)
			return (int32_t)(i + 1 + length);
	}

	return -1;
}

/*! Match "\s+(?!\S)": a whitespace run, less its last character when more follows. */
int32_t matchInteriorWhitespace(const CodePointView& view, size_t i)
{
	if (!isWhitespace(view[i]))
		return -1;

	size_t k = i;
	while (k < view.size() && isWhitespace(view[k]))
		++k;

	if (k >= view.size())
		return (int32_t)k;

	// A lone space before a word belongs to that word, not to this run.
	if (k - i >= 2)
		return (int32_t)(k - 1);

	return -1;
}

int32_t matchWhitespace(const CodePointView& view, size_t i)
{
	if (!isWhitespace(view[i]))
		return -1;

	size_t k = i;
	while (k < view.size() && isWhitespace(view[k]))
		++k;

	return (int32_t)k;
}

/*! Split \a text the way the GPT-2 pre-tokenizer pattern would. */
int32_t matchGpt2(const CodePointView& view, size_t i)
{
	const size_t n = view.size();

	const int32_t contraction = matchContraction(view, i, false);
	if (contraction > 0)
		return contraction;

	// " ?\p{L}+", " ?\p{N}+" and " ?[^\s\p{L}\p{N}]+" all admit one leading space.
	{
		size_t k = (view[i] == ' ') ? i + 1 : i;
		if (k < n)
		{
			if (isLetter(view[k]))
			{
				while (k < n && isLetter(view[k]))
					++k;
				return (int32_t)k;
			}
			if (isDigit(view[k]))
			{
				while (k < n && isDigit(view[k]))
					++k;
				return (int32_t)k;
			}
			if (isSymbol(view[k]))
			{
				while (k < n && isSymbol(view[k]))
					++k;
				return (int32_t)k;
			}
		}
	}

	const int32_t interior = matchInteriorWhitespace(view, i);
	if (interior > 0)
		return interior;

	return matchWhitespace(view, i);
}

/*! Split \a text the way the Llama 3 and Qwen 2 pre-tokenizer patterns would.
 *
 * The two differ in one place only: Llama 3 writes the digit rule as
 * \p{N}{1,3} and Qwen 2 as \p{N}, so a run of digits becomes one token per
 * three digits or one token per digit. Everything else is shared.
 */
int32_t matchLlama3(const CodePointView& view, size_t i, uint32_t maximumDigits)
{
	const size_t n = view.size();

	const int32_t contraction = matchContraction(view, i, true);
	if (contraction > 0)
		return contraction;

	// "[^\r\n\p{L}\p{N}]?\p{L}+" admits any single leading character that is
	// neither a line break, a letter nor a digit.
	{
		size_t k = i;
		if (!isNewLine(view[k]) && !isLetter(view[k]) && !isDigit(view[k]))
			++k;
		if (k < n && isLetter(view[k]))
		{
			while (k < n && isLetter(view[k]))
				++k;
			return (int32_t)k;
		}
	}

	// "\p{N}{1,3}" for Llama 3, "\p{N}" for Qwen 2.
	if (isDigit(view[i]))
	{
		size_t k = i;
		while (k < n && k < i + maximumDigits && isDigit(view[k]))
			++k;
		return (int32_t)k;
	}

	// " ?[^\s\p{L}\p{N}]+[\r\n]*"
	{
		size_t k = (view[i] == ' ') ? i + 1 : i;
		if (k < n && isSymbol(view[k]))
		{
			while (k < n && isSymbol(view[k]))
				++k;
			while (k < n && isNewLine(view[k]))
				++k;
			return (int32_t)k;
		}
	}

	// "\s*[\r\n]+" ends at the last line break inside the whitespace run.
	if (isWhitespace(view[i]))
	{
		size_t k = i;
		while (k < n && isWhitespace(view[k]))
			++k;

		size_t lastNewLine = k;
		while (lastNewLine > i && !isNewLine(view[lastNewLine - 1]))
			--lastNewLine;

		if (lastNewLine > i)
			return (int32_t)lastNewLine;
	}

	const int32_t interior = matchInteriorWhitespace(view, i);
	if (interior > 0)
		return interior;

	return matchWhitespace(view, i);
}

void preTokenize(const std::string& text, PreTokenizer preTokenizer, AlignedVector< std::string >& outWords)
{
	const CodePointView view(text);

	size_t i = 0;
	while (i < view.size())
	{
		int32_t end;
		switch (preTokenizer)
		{
		case PreTokenizer::Llama3:
			end = matchLlama3(view, i, 3);
			break;
		case PreTokenizer::Qwen2:
			end = matchLlama3(view, i, 1);
			break;
		default:
			end = matchGpt2(view, i);
			break;
		}

		if (end <= (int32_t)i)
			end = (int32_t)i + 1;

		const uint32_t from = view.getOffset(i);
		const uint32_t to = view.getOffset((size_t)end);
		outWords.push_back(text.substr(from, to - from));

		i = (size_t)end;
	}
}

/*! One run of text in the merge list, linked to its neighbours. */
struct Symbol
{
	int32_t prev;
	int32_t next;
	const char* text;
	size_t n;
};

/*! A candidate merge of two adjacent symbols. */
struct Bigram
{
	int32_t left;
	int32_t right;
	float priority;
	size_t size;

	bool operator < (const Bigram& rh) const
	{
		// Highest priority first; ties resolve leftmost first.
		return priority < rh.priority || (priority == rh.priority && left > rh.left);
	}
};

typedef std::priority_queue< Bigram > queue_t;

/*! Split \a text into one symbol per UTF-8 character. */
void buildSymbols(const std::string& text, AlignedVector< Symbol >& outSymbols)
{
	size_t i = 0;
	while (i < text.size())
	{
		const uint32_t length = getSequenceLength((uint8_t)text[i]);
		const size_t n = std::min((size_t)length, text.size() - i);

		Symbol& symbol = outSymbols.push_back();
		symbol.text = text.data() + i;
		symbol.n = n;
		symbol.prev = (int32_t)outSymbols.size() - 2;
		symbol.next = (i + n < text.size()) ? (int32_t)outSymbols.size() : -1;

		i += n;
	}
}

/*! Run the merge loop until no candidate remains. */
void mergeSymbols(AlignedVector< Symbol >& symbols, queue_t& queue, const std::function< void(int32_t, int32_t) >& addBigram)
{
	while (!queue.empty())
	{
		const Bigram bigram = queue.top();
		queue.pop();

		Symbol& left = symbols[bigram.left];
		Symbol& right = symbols[bigram.right];

		// Either side may already have been consumed by a better merge.
		if (left.n == 0 || right.n == 0 || left.n + right.n != bigram.size)
			continue;

		left.n += right.n;
		right.n = 0;

		left.next = right.next;
		if (right.next >= 0)
			symbols[right.next].prev = bigram.left;

		addBigram(left.prev, bigram.left);
		addBigram(bigram.left, left.next);
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.llm.Tokenizer", Tokenizer, Object)

bool Tokenizer::create(const GgufFile& file, const Vocabulary* vocabulary)
{
	if (vocabulary == nullptr)
		return false;

	m_vocabulary = vocabulary;

	const std::string pre = file.getString(L"tokenizer.ggml.pre", "default");
	if (pre == "qwen2")
		m_preTokenizer = PreTokenizer::Qwen2;
	else if (pre == "llama3" || pre == "llama-v3" || pre == "llama-bpe" || pre == "smaug-bpe")
		m_preTokenizer = PreTokenizer::Llama3;
	else
	{
		m_preTokenizer = PreTokenizer::Gpt2;

		// Every byte pair vocabulary was trained against one specific pattern,
		// and using another silently shifts where tokens begin. Say so rather
		// than let it look like a model problem later.
		if (vocabulary->getModel() == TokenizerModel::BytePair && pre != "default" && !pre.empty())
			log::warning << L"Unknown pre-tokenizer \"" << mbstows(pre) << L"\"; falling back on the GPT-2 pattern, which may tokenize differently than this model expects." << Endl;
	}

	m_addSpacePrefix = file.getBool(L"tokenizer.ggml.add_space_prefix", true);

	// Byte level alphabet, as introduced by GPT-2: printable ASCII and most of
	// Latin-1 stand for themselves, and the remaining bytes are lifted above
	// U+0100 so that every byte has a printable, reversible representation.
	uint32_t next = 256;
	for (uint32_t i = 0; i < 256; ++i)
	{
		const bool printable = (i >= 0x21 && i <= 0x7e) || (i >= 0xa1 && i <= 0xac) || (i >= 0xae && i <= 0xff);
		m_byteToCodePoint[i] = printable ? i : next++;
	}

	m_codePointToByte.resize(next + 1, -1);
	for (uint32_t i = 0; i < 256; ++i)
		m_codePointToByte[m_byteToCodePoint[i]] = (int32_t)i;

	return true;
}

void Tokenizer::encode(const std::string& text, bool addBeginOfSequence, bool parseSpecial, AlignedVector< int32_t >& outTokens) const
{
	if (addBeginOfSequence && m_vocabulary->getAddBeginOfSequence() && m_vocabulary->getBeginOfSequence() >= 0)
		outTokens.push_back(m_vocabulary->getBeginOfSequence());

	if (!parseSpecial)
	{
		encodeRaw(text, outTokens);
	}
	else
	{
		// Lift out any control or user defined token that appears literally,
		// so chat markup survives as the single token it must be.
		const AlignedVector< int32_t >& specials = m_vocabulary->getSpecialTokens();

		size_t i = 0;
		size_t spanStart = 0;
		while (i < text.size())
		{
			int32_t matched = -1;
			for (int32_t special : specials)
			{
				const std::string& marker = m_vocabulary->getTokenText(special);
				if (!marker.empty() && text.compare(i, marker.size(), marker) == 0)
				{
					matched = special;
					break;
				}
			}

			if (matched >= 0)
			{
				if (i > spanStart)
					encodeRaw(text.substr(spanStart, i - spanStart), outTokens);
				outTokens.push_back(matched);

				i += m_vocabulary->getTokenText(matched).size();
				spanStart = i;
			}
			else
				++i;
		}

		if (spanStart < text.size())
			encodeRaw(text.substr(spanStart), outTokens);
	}

	if (m_vocabulary->getAddEndOfSequence() && m_vocabulary->getEndOfSequence() >= 0)
		outTokens.push_back(m_vocabulary->getEndOfSequence());
}

std::string Tokenizer::decode(int32_t token) const
{
	// Chat markup is structure, not content; never render it.
	if (m_vocabulary->getTokenType(token) == TokenType::Control)
		return std::string();

	const std::string& text = m_vocabulary->getTokenText(token);

	if (m_vocabulary->getModel() == TokenizerModel::BytePair)
		return decodeByteLevel(text);

	if (m_vocabulary->getTokenType(token) == TokenType::Byte)
	{
		// "<0xNN>" stands for the single byte NN.
		if (text.size() == 6 && text.compare(0, 3, "<0x") == 0 && text[5] == '>')
		{
			const int32_t high = parseHexDigit(text[3]);
			const int32_t low = parseHexDigit(text[4]);
			if (high >= 0 && low >= 0)
				return std::string(1, (char)((high << 4) | low));
		}
	}

	std::string decoded;
	size_t i = 0;
	while (i < text.size())
	{
		if (text.compare(i, 3, c_spaceMarker) == 0)
		{
			decoded += ' ';
			i += 3;
		}
		else
			decoded += text[i++];
	}

	return decoded;
}

std::string Tokenizer::decode(const AlignedVector< int32_t >& tokens) const
{
	std::string decoded;
	for (int32_t token : tokens)
		decoded += decode(token);

	// SentencePiece encodes a leading marker that stands for no real space;
	// drop it again so a sequence round trips.
	if (m_vocabulary->getModel() == TokenizerModel::SentencePiece && m_addSpacePrefix)
	{
		if (!decoded.empty() && decoded[0] == ' ')
			decoded.erase(0, 1);
	}

	return decoded;
}

void Tokenizer::encodeRaw(const std::string& text, AlignedVector< int32_t >& outTokens) const
{
	if (text.empty())
		return;

	if (m_vocabulary->getModel() == TokenizerModel::SentencePiece)
		encodeSentencePiece(text, outTokens);
	else
		encodeBytePair(text, outTokens);
}

void Tokenizer::encodeSentencePiece(const std::string& text, AlignedVector< int32_t >& outTokens) const
{
	// SentencePiece sees a word marker in place of every space, and by
	// convention an extra one in front of the whole string.
	std::string prepared;
	if (m_addSpacePrefix)
		prepared = c_spaceMarker;

	for (char ch : text)
	{
		if (ch == ' ')
			prepared += c_spaceMarker;
		else
			prepared += ch;
	}

	AlignedVector< Symbol > symbols;
	buildSymbols(prepared, symbols);
	if (symbols.empty())
		return;

	queue_t queue;
	const auto addBigram = [&](int32_t left, int32_t right) {
		if (left < 0 || right < 0)
			return;

		const std::string merged(symbols[left].text, symbols[left].n + symbols[right].n);
		const int32_t token = m_vocabulary->findToken(merged);
		if (token < 0)
			return;

		queue.push({ left, right, m_vocabulary->getTokenScore(token), merged.size() });
	};

	for (int32_t i = 1; i < (int32_t)symbols.size(); ++i)
		addBigram(i - 1, i);

	mergeSymbols(symbols, queue, addBigram);

	for (int32_t i = 0; i >= 0; i = symbols[i].next)
	{
		const Symbol& symbol = symbols[i];
		if (symbol.n == 0)
			continue;

		const std::string piece(symbol.text, symbol.n);
		const int32_t token = m_vocabulary->findToken(piece);
		if (token >= 0)
			outTokens.push_back(token);
		else
			emitUnknown(piece, outTokens);
	}
}

void Tokenizer::encodeBytePair(const std::string& text, AlignedVector< int32_t >& outTokens) const
{
	AlignedVector< std::string > words;
	preTokenize(text, m_preTokenizer, words);

	for (const auto& word : words)
	{
		// Each byte becomes one printable code point, so a symbol is one byte.
		const std::string encoded = encodeByteLevel(word);

		AlignedVector< Symbol > symbols;
		buildSymbols(encoded, symbols);
		if (symbols.empty())
			continue;

		queue_t queue;
		const auto addBigram = [&](int32_t left, int32_t right) {
			if (left < 0 || right < 0)
				return;

			const std::string leftText(symbols[left].text, symbols[left].n);
			const std::string rightText(symbols[right].text, symbols[right].n);

			const int32_t rank = m_vocabulary->findMergeRank(leftText, rightText);
			if (rank < 0)
				return;

			// Merges are ordered by rank, lowest first.
			queue.push({ left, right, -(float)rank, leftText.size() + rightText.size() });
		};

		for (int32_t i = 1; i < (int32_t)symbols.size(); ++i)
			addBigram(i - 1, i);

		mergeSymbols(symbols, queue, addBigram);

		for (int32_t i = 0; i >= 0; i = symbols[i].next)
		{
			const Symbol& symbol = symbols[i];
			if (symbol.n == 0)
				continue;

			const std::string piece(symbol.text, symbol.n);
			const int32_t token = m_vocabulary->findToken(piece);
			if (token >= 0)
				outTokens.push_back(token);
			else
				emitUnknown(decodeByteLevel(piece), outTokens);
		}
	}
}

void Tokenizer::emitUnknown(const std::string& text, AlignedVector< int32_t >& outTokens) const
{
	// Fall back to one token per byte. SentencePiece spells those "<0xNN>";
	// byte pair vocabularies spell them in the byte level alphabet.
	for (char ch : text)
	{
		int32_t token = m_vocabulary->getByteToken((uint8_t)ch);

		if (token < 0 && m_vocabulary->getModel() == TokenizerModel::BytePair)
		{
			std::string piece;
			appendCodePoint(piece, m_byteToCodePoint[(uint8_t)ch]);
			token = m_vocabulary->findToken(piece);
		}

		if (token < 0)
			token = m_vocabulary->getUnknown();

		if (token >= 0)
			outTokens.push_back(token);
	}
}

std::string Tokenizer::encodeByteLevel(const std::string& text) const
{
	std::string encoded;
	for (char ch : text)
		appendCodePoint(encoded, m_byteToCodePoint[(uint8_t)ch]);
	return encoded;
}

std::string Tokenizer::decodeByteLevel(const std::string& text) const
{
	const CodePointView view(text);

	std::string decoded;
	for (size_t i = 0; i < view.size(); ++i)
	{
		const uint32_t codePoint = view[i];
		const int32_t byte = (codePoint < m_codePointToByte.size()) ? m_codePointToByte[codePoint] : -1;
		if (byte >= 0)
			decoded += (char)byte;
		else
			appendCodePoint(decoded, codePoint);
	}

	return decoded;
}

}
