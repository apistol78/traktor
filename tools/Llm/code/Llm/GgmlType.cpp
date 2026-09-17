/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/GgmlType.h"

#include "Core/Misc/String.h"

namespace traktor::llm
{
namespace
{

struct TypeDesc
{
	GgmlType type;
	uint32_t blockSize;
	uint32_t blockBytes;
	bool supported;
	const wchar_t* name;
};

// Block byte counts follow the GGUF tensor encoding; see Quant.cpp for the
// layout of each block.
const TypeDesc c_typeDescs[] = {
	{ GgmlType::F32, 1, 4, true, L"F32" },
	{ GgmlType::F16, 1, 2, true, L"F16" },
	{ GgmlType::Q4_0, 32, 18, true, L"Q4_0" },
	{ GgmlType::Q4_1, 32, 20, false, L"Q4_1" },
	{ GgmlType::Q5_0, 32, 22, false, L"Q5_0" },
	{ GgmlType::Q5_1, 32, 24, false, L"Q5_1" },
	{ GgmlType::Q8_0, 32, 34, true, L"Q8_0" },
	{ GgmlType::Q8_1, 32, 36, false, L"Q8_1" },
	{ GgmlType::Q2_K, 256, 84, false, L"Q2_K" },
	{ GgmlType::Q3_K, 256, 110, false, L"Q3_K" },
	{ GgmlType::Q4_K, 256, 144, true, L"Q4_K" },
	{ GgmlType::Q5_K, 256, 176, false, L"Q5_K" },
	{ GgmlType::Q6_K, 256, 210, true, L"Q6_K" },
	{ GgmlType::Q8_K, 256, 292, false, L"Q8_K" }
};

const TypeDesc* findDesc(GgmlType type)
{
	for (const auto& desc : c_typeDescs)
	{
		if (desc.type == type)
			return &desc;
	}
	return nullptr;
}

}

uint32_t getBlockSize(GgmlType type)
{
	const TypeDesc* desc = findDesc(type);
	return desc != nullptr ? desc->blockSize : 0;
}

uint32_t getBlockBytes(GgmlType type)
{
	const TypeDesc* desc = findDesc(type);
	return desc != nullptr ? desc->blockBytes : 0;
}

bool isSupported(GgmlType type)
{
	const TypeDesc* desc = findDesc(type);
	return desc != nullptr ? desc->supported : false;
}

std::wstring getTypeName(GgmlType type)
{
	const TypeDesc* desc = findDesc(type);
	if (desc != nullptr)
		return desc->name;
	else
		return L"Unknown (" + toString((uint32_t)type) + L")";
}

uint64_t getStorageSize(GgmlType type, uint64_t elementCount)
{
	const TypeDesc* desc = findDesc(type);
	if (desc == nullptr)
		return 0;
	return (elementCount / desc->blockSize) * desc->blockBytes;
}

}
