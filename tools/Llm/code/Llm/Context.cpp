/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/Context.h"

#include "Core/Log/Log.h"
#include "Llm/GgufFile.h"
#include "Llm/Model.h"
#include "Llm/Ops.h"
#include "Llm/Quant.h"

#include <cmath>

namespace traktor::llm
{
namespace
{

void addBias(float* x, const AlignedVector< float >& bias, int32_t n)
{
	if (bias.size() != (size_t)n)
		return;
	for (int32_t i = 0; i < n; ++i)
		x[i] += bias[i];
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.llm.Context", Context, Object)

bool Context::create(const Model* model, int32_t contextLength)
{
	if (model == nullptr)
		return false;

	const ModelParameters& parameters = model->getParameters();

	if (contextLength <= 0)
		contextLength = parameters.contextLength;
	if (contextLength <= 0)
		return false;

	// Positions beyond training are not merely slower, they are outside what
	// the rotary embedding was fitted for. Clamp quietly; whether asking for
	// more than this was a mistake is something only the caller knows.
	if (contextLength > parameters.contextLength && parameters.contextLength > 0)
		contextLength = parameters.contextLength;

	m_model = model;
	m_contextLength = contextLength;

	m_x.resize(parameters.embeddingLength, 0.0f);
	m_xb.resize(parameters.embeddingLength, 0.0f);
	m_xb2.resize(parameters.embeddingLength, 0.0f);
	m_hb.resize(parameters.feedForwardLength, 0.0f);
	m_hb2.resize(parameters.feedForwardLength, 0.0f);
	m_query.resize(parameters.embeddingLength, 0.0f);
	m_attention.resize((size_t)parameters.headCount * contextLength, 0.0f);
	m_logits.resize(parameters.vocabularyCount, 0.0f);

	// The cache is the single largest allocation here and it grows linearly
	// with the context, so say how big it is before asking for it rather than
	// leaving a failure to speak for itself.
	const size_t cacheSize = (size_t)parameters.layerCount * contextLength * parameters.keyValueLength;
	log::info << L"Key/value cache: " << contextLength << L" positions, " << (int32_t)((int64_t)cacheSize * 2 * sizeof(float) / (1024 * 1024)) << L" MiB." << Endl;

	m_keyCache.resize(cacheSize, 0.0f);
	m_valueCache.resize(cacheSize, 0.0f);

	reset();
	return true;
}

void Context::destroy()
{
	m_keyCache.clear();
	m_valueCache.clear();
	m_logits.clear();
	m_attention.clear();
	m_query.clear();
	m_hb2.clear();
	m_hb.clear();
	m_xb2.clear();
	m_xb.clear();
	m_x.clear();
	m_model = nullptr;
	m_contextLength = 0;
	m_position = 0;
	m_logitsValid = false;
}

void Context::reset()
{
	// Nothing past the write cursor is ever read, so the cache needs no
	// clearing; dropping the cursor is enough.
	m_position = 0;
	m_logitsValid = false;
}

void Context::rewind(int32_t position)
{
	if (position < 0)
		position = 0;
	if (position > m_position)
		return;

	m_position = position;
	m_logitsValid = false;
}

bool Context::evaluate(int32_t token, bool computeLogits)
{
	if (!m_model || m_position >= m_contextLength)
		return false;

	const ModelParameters& parameters = m_model->getParameters();
	const int32_t embed = parameters.embeddingLength;
	const int32_t kv = parameters.keyValueLength;
	const int32_t ff = parameters.feedForwardLength;
	const int32_t position = m_position;

	if (token < 0 || token >= parameters.vocabularyCount)
		return false;

	m_logitsValid = false;

	// Embedding lookup is a single row of the token table.
	const GgufTensor* embedding = m_model->getTokenEmbedding();
	const uint64_t rowBytes = getStorageSize(embedding->type, embed);
	dequantize(embedding->type, (const uint8_t*)embedding->data + (uint64_t)token * rowBytes, m_x.ptr(), embed);

	for (int32_t l = 0; l < parameters.layerCount; ++l)
	{
		const ModelLayer& layer = m_model->getLayer(l);

		float* key = m_keyCache.ptr() + ((size_t)l * m_contextLength + position) * kv;
		float* value = m_valueCache.ptr() + ((size_t)l * m_contextLength + position) * kv;

		rmsNorm(m_xb.ptr(), m_x.c_ptr(), layer.attentionNorm.c_ptr(), embed, parameters.rmsNormEpsilon);

		matmul(m_query.ptr(), *layer.attentionQ, m_xb.c_ptr(), embed, embed);
		matmul(key, *layer.attentionK, m_xb.c_ptr(), embed, kv);
		matmul(value, *layer.attentionV, m_xb.c_ptr(), embed, kv);

		addBias(m_query.ptr(), layer.attentionQBias, embed);
		addBias(key, layer.attentionKBias, kv);
		addBias(value, layer.attentionVBias, kv);

		// Position enters the network here, and only here.
		const float* frequencyFactors = m_model->getRopeFrequencyFactors();
		rope(m_query.ptr(), parameters.headCount, parameters.headDim, parameters.ropeDim, position, parameters.ropeFreqBase, parameters.ropeFreqScale, parameters.ropeType, frequencyFactors);
		rope(key, parameters.headCountKv, parameters.headDim, parameters.ropeDim, position, parameters.ropeFreqBase, parameters.ropeFreqScale, parameters.ropeType, frequencyFactors);

		attend(l, position);

		matmul(m_xb2.ptr(), *layer.attentionOutput, m_xb.c_ptr(), embed, embed);
		addTo(m_x.ptr(), m_xb2.c_ptr(), embed);

		rmsNorm(m_xb.ptr(), m_x.c_ptr(), layer.feedForwardNorm.c_ptr(), embed, parameters.rmsNormEpsilon);

		matmul(m_hb.ptr(), *layer.feedForwardGate, m_xb.c_ptr(), embed, ff);
		matmul(m_hb2.ptr(), *layer.feedForwardUp, m_xb.c_ptr(), embed, ff);
		siluMultiply(m_hb.ptr(), m_hb2.c_ptr(), ff);

		matmul(m_xb2.ptr(), *layer.feedForwardDown, m_hb.c_ptr(), ff, embed);
		addTo(m_x.ptr(), m_xb2.c_ptr(), embed);
	}

	++m_position;

	if (!computeLogits)
		return true;

	rmsNorm(m_xb.ptr(), m_x.c_ptr(), m_model->getOutputNorm().c_ptr(), embed, parameters.rmsNormEpsilon);
	matmul(m_logits.ptr(), *m_model->getOutput(), m_xb.c_ptr(), embed, parameters.vocabularyCount);

	m_logitsValid = true;
	return true;
}

int32_t Context::getLogitCount() const
{
	return m_model ? m_model->getParameters().vocabularyCount : 0;
}

int64_t Context::getCacheSize() const
{
	return (int64_t)(m_keyCache.size() + m_valueCache.size()) * sizeof(float);
}

void Context::attend(int32_t layer, int32_t position)
{
	const ModelParameters& parameters = m_model->getParameters();
	const int32_t headDim = parameters.headDim;
	const int32_t kv = parameters.keyValueLength;
	const int32_t groupSize = parameters.headCount / parameters.headCountKv;
	const float scale = 1.0f / std::sqrt((float)headDim);
	const int32_t length = position + 1;

	const float* keyBase = m_keyCache.c_ptr() + (size_t)layer * m_contextLength * kv;
	const float* valueBase = m_valueCache.c_ptr() + (size_t)layer * m_contextLength * kv;

	float* query = m_query.ptr();
	float* out = m_xb.ptr();
	float* attention = m_attention.ptr();
	const int32_t contextLength = m_contextLength;

	parallelFor(parameters.headCount, headDim * length, [=](uint32_t begin, uint32_t end) {
		for (uint32_t h = begin; h < end; ++h)
		{
			// Grouped query attention: several query heads share one cache head.
			const int32_t kvHead = (int32_t)h / groupSize;
			const int32_t channel = kvHead * headDim;

			const float* q = query + h * headDim;
			float* scores = attention + (size_t)h * contextLength;

			for (int32_t t = 0; t < length; ++t)
				scores[t] = dotF32(q, keyBase + (size_t)t * kv + channel, headDim) * scale;

			softmax(scores, length);

			float* result = out + h * headDim;
			for (int32_t i = 0; i < headDim; ++i)
				result[i] = 0.0f;

			for (int32_t t = 0; t < length; ++t)
			{
				const float weight = scores[t];
				const float* v = valueBase + (size_t)t * kv + channel;
				for (int32_t i = 0; i < headDim; ++i)
					result[i] += weight * v[i];
			}
		}
	});
}

}
