/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/Ops.h"

#include "Core/Containers/StaticVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Llm/GgufFile.h"
#include "Llm/Quant.h"

#include <algorithm>
#include <cmath>

namespace traktor::llm
{
namespace
{

const uint32_t c_maximumTasks = 32;

// Below this much arithmetic the scheduling costs more than the split saves.
const uint32_t c_inlineThreshold = 16 * 1024;

}

void parallelFor(uint32_t count, uint32_t costPerItem, const std::function< void(uint32_t, uint32_t) >& fn)
{
	if (count == 0)
		return;

	JobManager& jobManager = JobManager::getInstance();

	uint32_t taskCount = jobManager.getWorkerCount() + 1;
	taskCount = std::min(taskCount, c_maximumTasks);
	taskCount = std::min(taskCount, count);

	if (taskCount <= 1 || (uint64_t)count * costPerItem < c_inlineThreshold)
	{
		fn(0, count);
		return;
	}

	StaticVector< Job::task_t, c_maximumTasks > tasks;
	const uint32_t chunk = (count + taskCount - 1) / taskCount;

	for (uint32_t begin = 0; begin < count; begin += chunk)
	{
		const uint32_t end = std::min(begin + chunk, count);
		tasks.push_back([&fn, begin, end]() {
			fn(begin, end);
		});
	}

	// fork runs one task on this thread, so the caller must not itself be a
	// worker; generation always runs on a thread of its own.
	jobManager.fork(tasks.c_ptr(), tasks.size());
}

void matmul(float* outResult, const GgufTensor& weight, const float* x, uint32_t n, uint32_t d)
{
	const GgmlType type = weight.type;
	const uint8_t* base = (const uint8_t*)weight.data;
	const uint64_t rowBytes = getStorageSize(type, n);

	parallelFor(d, n, [=](uint32_t begin, uint32_t end) {
		for (uint32_t j = begin; j < end; ++j)
			outResult[j] = dotRow(type, base + j * rowBytes, x, n);
	});
}

void rmsNorm(float* outResult, const float* x, const float* weight, uint32_t n, float epsilon)
{
	// Accumulate in double; at 4096 channels a float sum of squares loses
	// enough precision to visibly shift the activations.
	double sum = 0.0;
	for (uint32_t i = 0; i < n; ++i)
		sum += (double)x[i] * (double)x[i];

	const float scale = 1.0f / std::sqrt((float)(sum / n) + epsilon);

	for (uint32_t i = 0; i < n; ++i)
		outResult[i] = x[i] * scale * weight[i];
}

void softmax(float* x, uint32_t n)
{
	if (n == 0)
		return;

	float maximum = x[0];
	for (uint32_t i = 1; i < n; ++i)
		maximum = std::max(maximum, x[i]);

	float sum = 0.0f;
	for (uint32_t i = 0; i < n; ++i)
	{
		x[i] = std::exp(x[i] - maximum);
		sum += x[i];
	}

	const float scale = (sum > 0.0f) ? 1.0f / sum : 0.0f;
	for (uint32_t i = 0; i < n; ++i)
		x[i] *= scale;
}

void siluMultiply(float* x, const float* y, uint32_t n)
{
	for (uint32_t i = 0; i < n; ++i)
		x[i] = (x[i] / (1.0f + std::exp(-x[i]))) * y[i];
}

void addTo(float* x, const float* y, uint32_t n)
{
	uint32_t i = 0;
	for (; i + 4 <= n; i += 4)
	{
		const Vector4 a = Vector4::loadUnaligned(x + i);
		const Vector4 b = Vector4::loadUnaligned(y + i);
		(a + b).storeUnaligned(x + i);
	}
	for (; i < n; ++i)
		x[i] += y[i];
}

void rope(float* x, uint32_t headCount, uint32_t headDim, uint32_t ropeDim, int32_t position, float freqBase, float freqScale, RopeType type, const float* frequencyFactors)
{
	const float scaledPosition = (float)position * freqScale;
	const uint32_t half = ropeDim / 2;

	for (uint32_t h = 0; h < headCount; ++h)
	{
		float* head = x + h * headDim;

		for (uint32_t i = 0; i < half; ++i)
		{
			float frequency = 1.0f / std::pow(freqBase, (float)(2 * i) / (float)ropeDim);

			// Dividing the frequency is the same as dividing the angle, and
			// keeps the factor out of the position arithmetic.
			if (frequencyFactors != nullptr)
				frequency /= frequencyFactors[i];

			const float theta = scaledPosition * frequency;
			const float cosTheta = std::cos(theta);
			const float sinTheta = std::sin(theta);

			// Normal rotates the pair (2i, 2i + 1); NeoX rotates (i, i + half).
			const uint32_t a = (type == RopeType::Normal) ? (2 * i) : i;
			const uint32_t b = (type == RopeType::Normal) ? (2 * i + 1) : (i + half);

			const float x0 = head[a];
			const float x1 = head[b];
			head[a] = x0 * cosTheta - x1 * sinTheta;
			head[b] = x0 * sinTheta + x1 * cosTheta;
		}
	}
}

}
