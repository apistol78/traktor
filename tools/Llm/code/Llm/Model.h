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
#include "Llm/Ops.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_LLM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Path;

}

namespace traktor::llm
{

class ChatTemplate;
class GgufFile;
class GgufTensor;
class Tokenizer;
class Vocabulary;

/*! Shape and hyper parameters of a decoder only transformer. */
class T_DLLCLASS ModelParameters
{
public:
	int32_t contextLength = 0;
	int32_t embeddingLength = 0;
	int32_t layerCount = 0;
	int32_t feedForwardLength = 0;
	int32_t headCount = 0;
	int32_t headCountKv = 0;
	int32_t headDim = 0;
	int32_t keyValueLength = 0; //!< headDim * headCountKv, the width of one cache entry.
	int32_t ropeDim = 0;
	int32_t vocabularyCount = 0;
	float rmsNormEpsilon = 1e-5f;
	float ropeFreqBase = 10000.0f;
	float ropeFreqScale = 1.0f;
	RopeType ropeType = RopeType::Normal;
};

/*! Weights of one transformer block.
 *
 * Large matrices stay in the mapping and are referenced; the vectors are
 * small enough that decoding them once at load time is cheaper than
 * decoding them on every token.
 */
class T_DLLCLASS ModelLayer
{
public:
	const GgufTensor* attentionQ = nullptr;
	const GgufTensor* attentionK = nullptr;
	const GgufTensor* attentionV = nullptr;
	const GgufTensor* attentionOutput = nullptr;
	const GgufTensor* feedForwardGate = nullptr;
	const GgufTensor* feedForwardUp = nullptr;
	const GgufTensor* feedForwardDown = nullptr;
	AlignedVector< float > attentionNorm;
	AlignedVector< float > feedForwardNorm;
	AlignedVector< float > attentionQBias;
	AlignedVector< float > attentionKBias;
	AlignedVector< float > attentionVBias;
};

/*! A loaded model: weights, vocabulary and chat markup.
 * \ingroup Llm
 *
 * Immutable once created, and safe to share between contexts. All
 * per-conversation state lives in Context instead.
 */
class T_DLLCLASS Model : public Object
{
	T_RTTI_CLASS;

public:
	virtual ~Model();

	bool create(const Path& fileName);

	void destroy();

	const ModelParameters& getParameters() const { return m_parameters; }

	const ModelLayer& getLayer(int32_t index) const { return m_layers[index]; }

	const GgufTensor* getTokenEmbedding() const { return m_tokenEmbedding; }

	const GgufTensor* getOutput() const { return m_output; }

	const AlignedVector< float >& getOutputNorm() const { return m_outputNorm; }

	/*! Per channel pair rotary divisors, or null if the model carries none.
	 *
	 * Present for Llama 3.1 and later, which stretch only the long wavelength
	 * channels instead of scaling the position.
	 */
	const float* getRopeFrequencyFactors() const { return m_ropeFrequencyFactors.empty() ? nullptr : m_ropeFrequencyFactors.c_ptr(); }

	/*! How the model's rotary embedding is scaled, for diagnostics. */
	const std::wstring& getRopeScaling() const { return m_ropeScaling; }

	const GgufFile* getFile() const { return m_file; }

	const Vocabulary* getVocabulary() const { return m_vocabulary; }

	const Tokenizer* getTokenizer() const { return m_tokenizer; }

	const ChatTemplate* getChatTemplate() const { return m_chatTemplate; }

	/*! One line summary for the user interface. */
	std::wstring getDescription() const;

private:
	Ref< GgufFile > m_file;
	Ref< Vocabulary > m_vocabulary;
	Ref< Tokenizer > m_tokenizer;
	Ref< ChatTemplate > m_chatTemplate;
	ModelParameters m_parameters;
	AlignedVector< ModelLayer > m_layers;
	const GgufTensor* m_tokenEmbedding = nullptr;
	const GgufTensor* m_output = nullptr;
	AlignedVector< float > m_outputNorm;
	AlignedVector< float > m_ropeFrequencyFactors;
	std::wstring m_ropeScaling;
	std::wstring m_name;

	bool readParameters();

	bool bindTensors();
};

}
