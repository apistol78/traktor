/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/SelfTest.h"

#include "Core/Io/Path.h"
#include "Core/Io/StringOutputStream.h"
#include "Llm/Utf8.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Llm/ChatTemplate.h"
#include "Llm/Context.h"
#include "Llm/Generator.h"
#include "Llm/GgufFile.h"
#include "Llm/Model.h"
#include "Llm/Quant.h"
#include "Llm/Sampler.h"
#include "Llm/Tokenizer.h"
#include "Llm/Vocabulary.h"

#include <algorithm>
#include <cmath>

namespace traktor::llm
{
namespace
{

const int32_t c_sampleCount = 8;
const char* const c_roundTripText = "Hello, world! The quick brown fox jumps over 12345 lazy dogs.";

/*! Print the first few decoded weights of every tensor.
 *
 * These are what an independent reader of the same bytes must reproduce;
 * a wrong shift or offset in a block layout shows up here immediately.
 */
void dumpTensors(const GgufFile& file)
{
	log::info << L"Tensors:" << Endl;

	AlignedVector< float > values(c_sampleCount, 0.0f);

	for (const auto& tensor : file.getTensors())
	{
		log::info << L"  " << tensor.name << L" [" << (int32_t)tensor.getRowLength() << L" x " << (int32_t)tensor.getRowCount() << L"] " << getTypeName(tensor.type);

		if (tensor.data == nullptr || !isSupported(tensor.type))
		{
			log::info << L" (not decoded)" << Endl;
			continue;
		}

		const uint32_t count = (uint32_t)std::min< uint64_t >(c_sampleCount, tensor.getRowLength());
		const uint32_t block = getBlockSize(tensor.type);

		// Decode whole blocks only; the sample is the head of the first one.
		AlignedVector< float > decoded(((count + block - 1) / block) * block, 0.0f);
		dequantize(tensor.type, tensor.data, decoded.ptr(), (uint32_t)decoded.size());

		log::info << L" =";
		for (uint32_t i = 0; i < count; ++i)
			log::info << L" " << toString(decoded[i], 6);
		log::info << Endl;
	}
}

/*! Drive the generator the way the user interface does.
 *
 * Runs two turns on the generator's own thread, polling for text, which is
 * the only path that exercises the worker, the streaming buffer and the
 * reuse of the cache across turns.
 */
bool checkGenerator(const Model* model, int32_t tokenCount, int32_t contextLength)
{
	Ref< Generator > generator = new Generator();
	if (!generator->create(model, contextLength))
	{
		log::error << L"Unable to create generator." << Endl;
		return false;
	}

	SamplerSettings settings;
	settings.temperature = 0.0f;
	settings.repeatPenalty = 1.0f;
	generator->setSamplerSettings(settings);

	AlignedVector< ChatMessage > conversation;
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();

	for (int32_t turn = 0; turn < 2; ++turn)
	{
		conversation.push_back(ChatMessage(ChatRole::User, (turn == 0) ? "Hello!" : "And again?"));

		if (!generator->begin(conversation, std::max(tokenCount, 1)))
		{
			log::error << L"Generator refused to start turn " << turn << L"." << Endl;
			return false;
		}

		std::string reply;
		Timer timeout;
		timeout.reset();

		while (generator->isBusy())
		{
			reply += generator->flushText();
			currentThread->sleep(5);

			if (timeout.getElapsedTime() > 60.0)
			{
				log::error << L"Generator did not finish within 60 s." << Endl;
				return false;
			}
		}

		// Whatever was produced between the last poll and the state change.
		reply += generator->flushText();

		const GeneratorState state = generator->getState();
		if (state != GeneratorState::Finished)
		{
			log::error << L"Generator ended in an unexpected state on turn " << turn << L": " << generator->getMessage() << Endl;
			return false;
		}

		int32_t used = 0;
		int32_t available = 0;
		generator->getContextUsage(used, available);

		log::info << L"Generator turn " << turn << L": prompt " << generator->getPromptTokenCount() << L", reply " << generator->getGeneratedTokenCount() << L" tokens, context " << used << L"/" << available << Endl;
		log::info << L"  \"" << widenUtf8(reply) << L"\"" << Endl;

		conversation.push_back(ChatMessage(ChatRole::Assistant, reply));
	}

	// The second turn shares its opening with the first, so it must have read
	// fewer prompt tokens than it has positions in the context.
	int32_t used = 0;
	int32_t available = 0;
	generator->getContextUsage(used, available);
	if (generator->getPromptTokenCount() >= used)
	{
		log::error << L"Second turn re-read the whole prompt; cache reuse is not working." << Endl;
		return false;
	}

	safeDestroy(generator);
	return true;
}

bool checkTokenizer(const Model& model)
{
	const Tokenizer* tokenizer = model.getTokenizer();

	AlignedVector< int32_t > tokens;
	tokenizer->encode(c_roundTripText, false, false, tokens);

	log::info << L"Tokenizer: " << (int32_t)tokens.size() << L" tokens for " << (int32_t)std::char_traits< char >::length(c_roundTripText) << L" bytes." << Endl;

	StringOutputStream ss;
	for (int32_t token : tokens)
		ss << token << L" ";
	log::info << L"  ids: " << ss.str() << Endl;

	const std::string decoded = tokenizer->decode(tokens);
	log::info << L"  text: \"" << widenUtf8(decoded) << L"\"" << Endl;

	if (decoded != c_roundTripText)
	{
		log::error << L"Tokenizer round trip failed; expected \"" << widenUtf8(c_roundTripText) << L"\"." << Endl;
		return false;
	}

	return true;
}

}

int32_t runSelfTest(const Path& fileName, int32_t tokenCount, int32_t contextLength)
{
	Ref< Model > model = new Model();
	if (!model->create(fileName))
		return 1;

	const ModelParameters& parameters = model->getParameters();

	log::info << L"Parameters:" << Endl;
	log::info << L"  context " << parameters.contextLength << L", embedding " << parameters.embeddingLength << L", layers " << parameters.layerCount << Endl;
	log::info << L"  heads " << parameters.headCount << L" (kv " << parameters.headCountKv << L"), head dim " << parameters.headDim << L", rope dim " << parameters.ropeDim << Endl;
	log::info << L"  feed forward " << parameters.feedForwardLength << L", vocabulary " << parameters.vocabularyCount << Endl;
	log::info << L"  rms epsilon " << toString(parameters.rmsNormEpsilon, 8) << L", rope base " << toString(parameters.ropeFreqBase, 2) << L", rope scale " << toString(parameters.ropeFreqScale, 4) << Endl;
	log::info << L"  rope " << (parameters.ropeType == RopeType::NeoX ? L"NeoX" : L"normal") << L", scaling " << model->getRopeScaling() << L", template " << model->getChatTemplate()->getKindName() << Endl;

	dumpTensors(*model->getFile());

	if (!checkTokenizer(*model))
		return 2;

	Ref< Context > context = new Context();
	if (!context->create(model, contextLength))
	{
		log::error << L"Unable to create context." << Endl;
		return 3;
	}

	log::info << L"Context: " << context->getContextLength() << L" positions, cache " << (int32_t)(context->getCacheSize() / (1024 * 1024)) << L" MiB." << Endl;

	AlignedVector< ChatMessage > messages;
	messages.push_back(ChatMessage(ChatRole::User, "Hello!"));

	AlignedVector< int32_t > tokens;
	model->getTokenizer()->encode(model->getChatTemplate()->format(messages), true, true, tokens);

	if (tokens.empty())
	{
		log::error << L"Prompt produced no tokens." << Endl;
		return 4;
	}

	Timer timer;
	timer.reset();

	for (size_t i = 0; i < tokens.size(); ++i)
	{
		if (!context->evaluate(tokens[i], i + 1 == tokens.size()))
		{
			log::error << L"Evaluation failed at prompt token " << (int32_t)i << L"." << Endl;
			return 5;
		}
	}

	const double promptTime = timer.getElapsedTime();
	log::info << L"Prompt: " << (int32_t)tokens.size() << L" tokens in " << toString(promptTime, 3) << L" s (" << toString(tokens.size() / std::max(promptTime, 1e-6), 1) << L" tok/s)." << Endl;

	// Summarize the logits after the prompt. Any change anywhere in the
	// forward pass moves these numbers, including changes far too small to
	// move the sampled token, which makes this the thing to diff against a
	// reference implementation or across a code change.
	{
		const float* logits = context->getLogits();
		if (logits == nullptr)
		{
			log::error << L"No logits after the prompt." << Endl;
			return 5;
		}

		double sum = 0.0;
		float maximum = logits[0];
		int32_t argmax = 0;

		for (int32_t i = 0; i < context->getLogitCount(); ++i)
		{
			sum += logits[i];
			if (logits[i] > maximum)
			{
				maximum = logits[i];
				argmax = i;
			}
		}

		log::info << L"Logits: sum " << toString(sum, 6) << L", max " << toString(maximum, 6) << L" at token " << argmax << L"." << Endl;
	}

	// Greedy, so the run is reproducible and a change in the numbers is a
	// change in the model, not in the draw.
	SamplerSettings settings;
	settings.temperature = 0.0f;
	settings.repeatPenalty = 1.0f;
	Ref< Sampler > sampler = new Sampler(settings);

	AlignedVector< int32_t > history = tokens;
	std::string generated;

	timer.reset();

	for (int32_t i = 0; i < tokenCount; ++i)
	{
		float* logits = context->getLogits();
		if (logits == nullptr)
		{
			log::error << L"No logits at generated token " << i << L"." << Endl;
			return 6;
		}

		// A single non finite logit means the forward pass has diverged, and
		// every token after it is meaningless.
		for (int32_t j = 0; j < context->getLogitCount(); ++j)
		{
			if (!std::isfinite(logits[j]))
			{
				log::error << L"Logit " << j << L" is not finite at generated token " << i << L"." << Endl;
				return 7;
			}
		}

		const int32_t token = sampler->sample(logits, context->getLogitCount(), history);
		if (token < 0)
		{
			log::error << L"Sampler produced no token." << Endl;
			return 8;
		}

		if (model->getVocabulary()->isEndOfGeneration(token))
		{
			log::info << L"End of generation after " << i << L" tokens." << Endl;
			break;
		}

		generated += model->getTokenizer()->decode(token);
		history.push_back(token);

		if (!context->evaluate(token, true))
		{
			log::info << L"Context full after " << i << L" tokens." << Endl;
			break;
		}
	}

	const double generateTime = timer.getElapsedTime();
	const int32_t generatedCount = (int32_t)history.size() - (int32_t)tokens.size();

	log::info << L"Generated: " << generatedCount << L" tokens in " << toString(generateTime, 3) << L" s (" << toString(generatedCount / std::max(generateTime, 1e-6), 1) << L" tok/s)." << Endl;
	log::info << L"Output: \"" << widenUtf8(generated) << L"\"" << Endl;

	if (!checkGenerator(model, tokenCount, contextLength))
		return 9;

	log::info << L"Self test passed." << Endl;

	return 0;
}

}
