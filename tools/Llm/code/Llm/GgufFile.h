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
#include "Core/Containers/SmallMap.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Llm/GgmlType.h"

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

class IMappedFile;
class Path;

}

namespace traktor::llm
{

/*! Type tag of a GGUF metadata value.
 * \ingroup Llm
 *
 * Values are defined by the GGUF container and must not be renumbered.
 */
enum class GgufValueType : uint32_t
{
	UInt8 = 0,
	Int8 = 1,
	UInt16 = 2,
	Int16 = 3,
	UInt32 = 4,
	Int32 = 5,
	Float32 = 6,
	Bool = 7,
	String = 8,
	Array = 9,
	UInt64 = 10,
	Int64 = 11,
	Float64 = 12,
	Invalid = 0xffffffff
};

/*! A single metadata value read from the header of a GGUF file.
 * \ingroup Llm
 *
 * Numeric scalars are widened into both an integer and a floating point
 * field so callers need not care which width the file happened to use.
 * Strings are kept as raw UTF-8 since the tokenizer works on bytes.
 */
class T_DLLCLASS GgufValue
{
public:
	GgufValueType type = GgufValueType::Invalid;
	GgufValueType arrayType = GgufValueType::Invalid;
	int64_t integer = 0;
	double number = 0.0;
	std::string string;
	AlignedVector< double > numbers;
	AlignedVector< std::string > strings;

	bool isArray() const { return type == GgufValueType::Array; }
};

/*! Directory entry describing one tensor in a GGUF file.
 * \ingroup Llm
 *
 * \a dims are given innermost first, matching the container; for a weight
 * matrix dims[0] is the length of a row and dims[1] the number of rows.
 */
class T_DLLCLASS GgufTensor
{
public:
	std::wstring name;
	GgmlType type = GgmlType::Invalid;
	uint32_t dimCount = 0;
	uint64_t dims[4] = { 1, 1, 1, 1 };
	uint64_t offset = 0;
	const void* data = nullptr;

	uint64_t getElementCount() const { return dims[0] * dims[1] * dims[2] * dims[3]; }

	uint64_t getRowLength() const { return dims[0]; }

	uint64_t getRowCount() const { return dims[1] * dims[2] * dims[3]; }

	/*! Byte offset from \a data to the first element of row \a row. */
	uint64_t getRowOffset(uint64_t row) const { return getStorageSize(type, row * dims[0]); }

	/*! Pointer to the first element of row \a row. */
	const void* getRow(uint64_t row) const { return (const uint8_t*)data + getRowOffset(row); }
};

/*! Memory mapped GGUF model file.
 * \ingroup Llm
 *
 * Only the header is parsed eagerly; tensor payloads stay in the mapping
 * and are decoded on demand, so opening a multi gigabyte model costs
 * neither the time nor the resident memory of a copy.
 */
class T_DLLCLASS GgufFile : public Object
{
	T_RTTI_CLASS;

public:
	virtual ~GgufFile();

	/*! Map \a fileName and parse its header. */
	bool create(const Path& fileName);

	void destroy();

	const GgufValue* findValue(const std::wstring& key) const;

	int64_t getInteger(const std::wstring& key, int64_t defaultValue) const;

	double getNumber(const std::wstring& key, double defaultValue) const;

	bool getBool(const std::wstring& key, bool defaultValue) const;

	std::string getString(const std::wstring& key, const std::string& defaultValue) const;

	const GgufTensor* findTensor(const std::wstring& name) const;

	const AlignedVector< GgufTensor >& getTensors() const { return m_tensors; }

	const SmallMap< std::wstring, GgufValue >& getMetadata() const { return m_metadata; }

	/*! Value of "general.architecture", e.g. "llama" or "qwen2". */
	const std::wstring& getArchitecture() const { return m_architecture; }

	/*! Total size of the mapping, in bytes. */
	int64_t getFileSize() const;

private:
	Ref< IMappedFile > m_mappedFile;
	SmallMap< std::wstring, GgufValue > m_metadata;
	SmallMap< std::wstring, uint32_t > m_tensorIndex;
	AlignedVector< GgufTensor > m_tensors;
	std::wstring m_architecture;
};

}
