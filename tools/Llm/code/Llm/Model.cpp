/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/Model.h"

#include "Core/Io/Path.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Llm/ChatTemplate.h"
#include "Llm/GgufFile.h"
#include "Llm/Quant.h"
#include "Llm/Tokenizer.h"
#include "Llm/Vocabulary.h"

namespace traktor::llm
{
namespace
{

/*! Architectures whose graph is exactly the one implemented here.
 *
 * Everything on this list is RMS normalized, rotary, SwiGLU and grouped
 * query; they differ only in whether the attention projections carry a bias
 * and in how rotary embedding pairs up channels.
 */
struct ArchitectureDesc
{
	const wchar_t* name;
	RopeType ropeType;
};

const ArchitectureDesc c_architectures[] = {
	{ L"llama", RopeType::Normal },
	{ L"mistral", RopeType::Normal },
	{ L"qwen2", RopeType::NeoX }
};

const ArchitectureDesc* findArchitecture(const std::wstring& name)
{
	for (const auto& desc : c_architectures)
	{
		if (name == desc.name)
			return &desc;
	}
	return nullptr;
}

std::wstring formatSize(int64_t bytes)
{
	const double mib = (double)bytes / (1024.0 * 1024.0);
	if (mib >= 1024.0)
		return toString(mib / 1024.0, 2) + L" GiB";
	else
		return toString(mib, 1) + L" MiB";
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.llm.Model", Model, Object)

Model::~Model()
{
	destroy();
}

bool Model::create(const Path& fileName)
{
	m_file = new GgufFile();
	if (!m_file->create(fileName))
	{
		destroy();
		return false;
	}

	if (!readParameters())
	{
		destroy();
		return false;
	}

	if (!bindTensors())
	{
		destroy();
		return false;
	}

	m_vocabulary = new Vocabulary();
	if (!m_vocabulary->create(*m_file))
	{
		destroy();
		return false;
	}

	m_tokenizer = new Tokenizer();
	if (!m_tokenizer->create(*m_file, m_vocabulary))
	{
		destroy();
		return false;
	}

	m_chatTemplate = new ChatTemplate();
	if (!m_chatTemplate->create(*m_file, m_vocabulary))
	{
		destroy();
		return false;
	}

	// The token table is authoritative; a mismatch means the output matrix
	// and the vocabulary disagree and every sampled token would be wrong.
	if (m_vocabulary->getTokenCount() != m_parameters.vocabularyCount)
	{
		log::error << L"Model declares " << m_parameters.vocabularyCount << L" output classes but the vocabulary has " << m_vocabulary->getTokenCount() << L" tokens." << Endl;
		destroy();
		return false;
	}

	m_name = mbstows(m_file->getString(L"general.name", ""));
	if (m_name.empty())
		m_name = Path(fileName).getFileNameNoExtension();

	log::info << L"Rotary scaling: " << m_ropeScaling << L"." << Endl;
	log::info << L"Loaded " << getDescription() << L"." << Endl;
	return true;
}

void Model::destroy()
{
	m_layers.clear();
	m_outputNorm.clear();
	m_ropeFrequencyFactors.clear();
	m_ropeScaling.clear();
	m_tokenEmbedding = nullptr;
	m_output = nullptr;
	m_chatTemplate = nullptr;
	m_tokenizer = nullptr;
	m_vocabulary = nullptr;
	m_file = nullptr;
}

std::wstring Model::getDescription() const
{
	StringOutputStream ss;
	ss << m_name;
	ss << L" (" << m_parameters.layerCount << L" layers, " << m_parameters.embeddingLength << L" wide, ";
	ss << m_parameters.headCount << L"/" << m_parameters.headCountKv << L" heads, ";
	ss << m_parameters.contextLength << L" ctx, ";
	ss << formatSize(m_file ? m_file->getFileSize() : 0) << L")";
	return ss.str();
}

bool Model::readParameters()
{
	const std::wstring architecture = m_file->getArchitecture();
	const ArchitectureDesc* desc = findArchitecture(architecture);
	if (desc == nullptr)
	{
		log::error << L"Unsupported architecture \"" << architecture << L"\"; this engine implements";
		for (const auto& supported : c_architectures)
			log::error << L" " << supported.name;
		log::error << L"." << Endl;
		return false;
	}

	m_parameters.ropeType = desc->ropeType;

	const std::wstring prefix = architecture + L".";
	m_parameters.contextLength = (int32_t)m_file->getInteger(prefix + L"context_length", 0);
	m_parameters.embeddingLength = (int32_t)m_file->getInteger(prefix + L"embedding_length", 0);
	m_parameters.layerCount = (int32_t)m_file->getInteger(prefix + L"block_count", 0);
	m_parameters.feedForwardLength = (int32_t)m_file->getInteger(prefix + L"feed_forward_length", 0);
	m_parameters.headCount = (int32_t)m_file->getInteger(prefix + L"attention.head_count", 0);
	m_parameters.headCountKv = (int32_t)m_file->getInteger(prefix + L"attention.head_count_kv", m_parameters.headCount);
	m_parameters.rmsNormEpsilon = (float)m_file->getNumber(prefix + L"attention.layer_norm_rms_epsilon", 1e-5);
	m_parameters.ropeFreqBase = (float)m_file->getNumber(prefix + L"rope.freq_base", 10000.0);

	if (m_parameters.embeddingLength <= 0 || m_parameters.layerCount <= 0 || m_parameters.headCount <= 0)
	{
		log::error << L"Model header is missing required shape parameters." << Endl;
		return false;
	}

	if ((m_parameters.embeddingLength % m_parameters.headCount) != 0)
	{
		log::error << L"Embedding length " << m_parameters.embeddingLength << L" is not divisible by " << m_parameters.headCount << L" heads." << Endl;
		return false;
	}

	if (m_parameters.headCountKv <= 0 || (m_parameters.headCount % m_parameters.headCountKv) != 0)
	{
		log::error << L"Head count " << m_parameters.headCount << L" is not a multiple of the " << m_parameters.headCountKv << L" key/value heads." << Endl;
		return false;
	}

	m_parameters.headDim = m_parameters.embeddingLength / m_parameters.headCount;
	m_parameters.keyValueLength = m_parameters.headDim * m_parameters.headCountKv;
	m_parameters.ropeDim = (int32_t)m_file->getInteger(prefix + L"rope.dimension_count", m_parameters.headDim);

	if (m_parameters.ropeDim <= 0 || m_parameters.ropeDim > m_parameters.headDim || (m_parameters.ropeDim % 2) != 0)
	{
		log::error << L"Invalid rotary dimension count " << m_parameters.ropeDim << L"." << Endl;
		return false;
	}

	// Rotary scaling. The type key is frequently absent, and llama.cpp reads
	// that as linear, so match it: a bare factor means linear.
	//
	// Llama 3.1 and later are the exception that matters here. Their converter
	// writes neither key, and instead bakes the correction into a
	// "rope_freqs.weight" tensor which bindTensors picks up; a model using
	// that path is left at a position scale of one on purpose.
	const std::string scalingType = m_file->getString(prefix + L"rope.scaling.type", "linear");
	const double ropeScale = m_file->getNumber(prefix + L"rope.scaling.factor", 0.0);

	m_parameters.ropeFreqScale = 1.0f;
	m_ropeScaling = L"none";

	if (scalingType == "linear")
	{
		if (ropeScale > 0.0)
		{
			m_parameters.ropeFreqScale = (float)(1.0 / ropeScale);
			m_ropeScaling = L"linear x" + toString(ropeScale, 2);
		}
	}
	else if (scalingType == "none")
	{
		// Explicitly unscaled; nothing to do.
	}
	else
	{
		// YaRN and LongRoPE reshape the frequencies in ways this engine does
		// not implement. Running them unscaled is wrong, but quietly applying
		// a linear factor in their place would be worse.
		log::warning << L"Rotary scaling \"" << mbstows(scalingType) << L"\" is not implemented; positions will be left unscaled and long context quality will suffer." << Endl;
		m_ropeScaling = mbstows(scalingType) + L" (unsupported)";
	}

	return true;
}

bool Model::bindTensors()
{
	bool valid = true;

	// Resolve a tensor by name, reporting anything missing or stored in a
	// format this build cannot decode.
	const auto bind = [&](const std::wstring& name, bool required) -> const GgufTensor* {
		const GgufTensor* tensor = m_file->findTensor(name);
		if (tensor == nullptr)
		{
			if (required)
			{
				log::error << L"Model is missing tensor \"" << name << L"\"." << Endl;
				valid = false;
			}
			return nullptr;
		}

		if (!isSupported(tensor->type))
		{
			log::error << L"Tensor \"" << name << L"\" is stored as " << getTypeName(tensor->type) << L", which this build cannot decode." << Endl;
			valid = false;
			return nullptr;
		}

		return tensor;
	};

	// Decode a small tensor once, rather than on every token.
	const auto bindVector = [&](const std::wstring& name, bool required, AlignedVector< float >& outValues) {
		const GgufTensor* tensor = bind(name, required);
		if (tensor == nullptr)
			return;

		outValues.resize((size_t)tensor->getElementCount());
		dequantize(tensor->type, tensor->data, outValues.ptr(), (uint32_t)tensor->getElementCount());
	};

	m_tokenEmbedding = bind(L"token_embd.weight", true);
	bindVector(L"output_norm.weight", true, m_outputNorm);

	// Llama 3.1 and later ship the rotary correction as a tensor rather than
	// as header fields; one divisor per rotated channel pair.
	bindVector(L"rope_freqs.weight", false, m_ropeFrequencyFactors);
	if (!m_ropeFrequencyFactors.empty())
	{
		const size_t expected = (size_t)(m_parameters.ropeDim / 2);
		if (m_ropeFrequencyFactors.size() != expected)
		{
			log::warning << L"\"rope_freqs.weight\" has " << (int32_t)m_ropeFrequencyFactors.size() << L" entries but " << (int32_t)expected << L" rotary channel pairs were expected; ignoring it." << Endl;
			m_ropeFrequencyFactors.clear();
		}
		else
			m_ropeScaling = L"per channel (rope_freqs)";
	}

	// Small models frequently tie the output matrix to the embedding table.
	m_output = bind(L"output.weight", false);
	if (m_output == nullptr)
		m_output = m_tokenEmbedding;

	m_layers.resize(m_parameters.layerCount);
	for (int32_t i = 0; i < m_parameters.layerCount; ++i)
	{
		const std::wstring prefix = L"blk." + toString(i) + L".";
		ModelLayer& layer = m_layers[i];

		bindVector(prefix + L"attn_norm.weight", true, layer.attentionNorm);
		bindVector(prefix + L"ffn_norm.weight", true, layer.feedForwardNorm);

		layer.attentionQ = bind(prefix + L"attn_q.weight", true);
		layer.attentionK = bind(prefix + L"attn_k.weight", true);
		layer.attentionV = bind(prefix + L"attn_v.weight", true);
		layer.attentionOutput = bind(prefix + L"attn_output.weight", true);

		layer.feedForwardGate = bind(prefix + L"ffn_gate.weight", true);
		layer.feedForwardUp = bind(prefix + L"ffn_up.weight", true);
		layer.feedForwardDown = bind(prefix + L"ffn_down.weight", true);

		// Qwen carries a bias on the attention projections; Llama does not.
		bindVector(prefix + L"attn_q.bias", false, layer.attentionQBias);
		bindVector(prefix + L"attn_k.bias", false, layer.attentionKBias);
		bindVector(prefix + L"attn_v.bias", false, layer.attentionVBias);
	}

	if (!valid)
		return false;

	m_parameters.vocabularyCount = (int32_t)m_tokenEmbedding->getRowCount();

	// Verify the shapes agree with the header before anything reads past a row.
	const auto check = [&](const GgufTensor* tensor, const wchar_t* what, int32_t rowLength, int32_t rowCount) {
		if (tensor == nullptr)
			return;
		if ((int32_t)tensor->getRowLength() != rowLength || (int32_t)tensor->getRowCount() != rowCount)
		{
			log::error << L"Tensor \"" << tensor->name << L"\" (" << what << L") is " << (int32_t)tensor->getRowLength() << L" by " << (int32_t)tensor->getRowCount() << L"; expected " << rowLength << L" by " << rowCount << L"." << Endl;
			valid = false;
		}
	};

	const int32_t embed = m_parameters.embeddingLength;
	const int32_t kv = m_parameters.keyValueLength;
	const int32_t ff = m_parameters.feedForwardLength;

	check(m_tokenEmbedding, L"token embedding", embed, m_parameters.vocabularyCount);

	for (int32_t i = 0; i < m_parameters.layerCount; ++i)
	{
		const ModelLayer& layer = m_layers[i];
		check(layer.attentionQ, L"query projection", embed, embed);
		check(layer.attentionK, L"key projection", embed, kv);
		check(layer.attentionV, L"value projection", embed, kv);
		check(layer.attentionOutput, L"attention output", embed, embed);
		check(layer.feedForwardGate, L"feed forward gate", embed, ff);
		check(layer.feedForwardUp, L"feed forward up", embed, ff);
		check(layer.feedForwardDown, L"feed forward down", ff, embed);
	}

	return valid;
}

}
