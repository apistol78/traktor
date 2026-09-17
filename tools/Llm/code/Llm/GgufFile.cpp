/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/GgufFile.h"

#include "Core/Io/FileSystem.h"
#include "Core/Io/IMappedFile.h"
#include "Core/Io/Path.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"

#include <cstring>

namespace traktor::llm
{
namespace
{

const uint32_t c_ggufMagic = 0x46554747;
const uint32_t c_minimumVersion = 2;
const uint32_t c_maximumVersion = 3;
const uint64_t c_defaultAlignment = 32;
const uint64_t c_maximumDimensions = 4;

// Sanity bounds; a header claiming more than this is treated as corrupt
// rather than being allowed to drive an allocation.
const uint64_t c_maximumEntries = 1u << 24;

/*! Bounds checked forward reader over the mapped file. */
class Cursor
{
public:
	Cursor(const uint8_t* base, uint64_t size)
		: m_base(base)
		, m_size(size)
	{
	}

	bool read(void* outData, uint64_t size)
	{
		if (m_position + size > m_size)
			return false;
		std::memcpy(outData, m_base + m_position, size);
		m_position += size;
		return true;
	}

	template < typename ValueType >
	bool read(ValueType& outValue)
	{
		return read(&outValue, sizeof(ValueType));
	}

	bool readString(std::string& outString)
	{
		uint64_t length;
		if (!read(length))
			return false;
		if (m_position + length > m_size)
			return false;
		outString.assign((const char*)(m_base + m_position), (size_t)length);
		m_position += length;
		return true;
	}

	bool skip(uint64_t size)
	{
		if (m_position + size > m_size)
			return false;
		m_position += size;
		return true;
	}

	uint64_t tell() const { return m_position; }

	uint64_t available() const { return m_size - m_position; }

private:
	const uint8_t* m_base;
	uint64_t m_size;
	uint64_t m_position = 0;
};

/*! Size in bytes of a fixed width GGUF scalar, or zero if it has none. */
uint64_t getScalarSize(GgufValueType type)
{
	switch (type)
	{
	case GgufValueType::UInt8:
	case GgufValueType::Int8:
	case GgufValueType::Bool:
		return 1;
	case GgufValueType::UInt16:
	case GgufValueType::Int16:
		return 2;
	case GgufValueType::UInt32:
	case GgufValueType::Int32:
	case GgufValueType::Float32:
		return 4;
	case GgufValueType::UInt64:
	case GgufValueType::Int64:
	case GgufValueType::Float64:
		return 8;
	default:
		return 0;
	}
}

/*! Read one fixed width scalar, widening it into both representations. */
bool readScalar(Cursor& cursor, GgufValueType type, int64_t& outInteger, double& outNumber)
{
	switch (type)
	{
	case GgufValueType::UInt8:
		{
			uint8_t v;
			if (!cursor.read(v))
				return false;
			outInteger = v;
			outNumber = (double)v;
		}
		break;

	case GgufValueType::Int8:
		{
			int8_t v;
			if (!cursor.read(v))
				return false;
			outInteger = v;
			outNumber = (double)v;
		}
		break;

	case GgufValueType::UInt16:
		{
			uint16_t v;
			if (!cursor.read(v))
				return false;
			outInteger = v;
			outNumber = (double)v;
		}
		break;

	case GgufValueType::Int16:
		{
			int16_t v;
			if (!cursor.read(v))
				return false;
			outInteger = v;
			outNumber = (double)v;
		}
		break;

	case GgufValueType::UInt32:
		{
			uint32_t v;
			if (!cursor.read(v))
				return false;
			outInteger = v;
			outNumber = (double)v;
		}
		break;

	case GgufValueType::Int32:
		{
			int32_t v;
			if (!cursor.read(v))
				return false;
			outInteger = v;
			outNumber = (double)v;
		}
		break;

	case GgufValueType::Float32:
		{
			float v;
			if (!cursor.read(v))
				return false;
			outInteger = (int64_t)v;
			outNumber = (double)v;
		}
		break;

	case GgufValueType::Bool:
		{
			uint8_t v;
			if (!cursor.read(v))
				return false;
			outInteger = (v != 0) ? 1 : 0;
			outNumber = (double)outInteger;
		}
		break;

	case GgufValueType::UInt64:
		{
			uint64_t v;
			if (!cursor.read(v))
				return false;
			outInteger = (int64_t)v;
			outNumber = (double)v;
		}
		break;

	case GgufValueType::Int64:
		{
			int64_t v;
			if (!cursor.read(v))
				return false;
			outInteger = v;
			outNumber = (double)v;
		}
		break;

	case GgufValueType::Float64:
		{
			double v;
			if (!cursor.read(v))
				return false;
			outInteger = (int64_t)v;
			outNumber = v;
		}
		break;

	default:
		return false;
	}

	return true;
}

bool readValue(Cursor& cursor, GgufValue& outValue)
{
	uint32_t rawType;
	if (!cursor.read(rawType))
		return false;

	outValue.type = (GgufValueType)rawType;

	if (outValue.type == GgufValueType::String)
		return cursor.readString(outValue.string);

	if (outValue.type == GgufValueType::Array)
	{
		uint32_t rawArrayType;
		uint64_t count;
		if (!cursor.read(rawArrayType) || !cursor.read(count))
			return false;

		outValue.arrayType = (GgufValueType)rawArrayType;
		if (count > c_maximumEntries)
			return false;

		if (outValue.arrayType == GgufValueType::String)
		{
			outValue.strings.resize((size_t)count);
			for (uint64_t i = 0; i < count; ++i)
			{
				if (!cursor.readString(outValue.strings[(size_t)i]))
					return false;
			}
			return true;
		}

		const uint64_t scalarSize = getScalarSize(outValue.arrayType);
		if (scalarSize == 0)
			return false;

		// Reject before resizing so a corrupt count cannot force a large
		// allocation for data the file does not actually contain.
		if (count * scalarSize > cursor.available())
			return false;

		outValue.numbers.resize((size_t)count);
		for (uint64_t i = 0; i < count; ++i)
		{
			int64_t integer;
			double number;
			if (!readScalar(cursor, outValue.arrayType, integer, number))
				return false;
			outValue.numbers[(size_t)i] = number;
		}
		return true;
	}

	return readScalar(cursor, outValue.type, outValue.integer, outValue.number);
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.llm.GgufFile", GgufFile, Object)

GgufFile::~GgufFile()
{
	destroy();
}

bool GgufFile::create(const Path& fileName)
{
	m_mappedFile = FileSystem::getInstance().map(fileName);
	if (!m_mappedFile)
	{
		log::error << L"Unable to open GGUF file \"" << fileName.getPathName() << L"\"." << Endl;
		return false;
	}

	const uint8_t* base = (const uint8_t*)m_mappedFile->getBase();
	const uint64_t size = (uint64_t)m_mappedFile->getSize();
	Cursor cursor(base, size);

	uint32_t magic;
	uint32_t version;
	uint64_t tensorCount;
	uint64_t metadataCount;

	if (!cursor.read(magic) || !cursor.read(version) || !cursor.read(tensorCount) || !cursor.read(metadataCount))
	{
		log::error << L"Malformed GGUF file; truncated header." << Endl;
		destroy();
		return false;
	}

	if (magic != c_ggufMagic)
	{
		log::error << L"Not a GGUF file; bad magic." << Endl;
		destroy();
		return false;
	}

	if (version < c_minimumVersion || version > c_maximumVersion)
	{
		log::error << L"Unsupported GGUF version " << version << L"; expected " << c_minimumVersion << L" to " << c_maximumVersion << L"." << Endl;
		destroy();
		return false;
	}

	if (tensorCount > c_maximumEntries || metadataCount > c_maximumEntries)
	{
		log::error << L"Malformed GGUF file; implausible header counts." << Endl;
		destroy();
		return false;
	}

	for (uint64_t i = 0; i < metadataCount; ++i)
	{
		std::string key;
		if (!cursor.readString(key))
		{
			log::error << L"Malformed GGUF file; truncated metadata key " << i << L"." << Endl;
			destroy();
			return false;
		}

		// Insert empty and fill in place; the vocabulary arrays are large
		// enough that copying a parsed value into the map would be wasteful.
		const std::wstring wideKey = mbstows(key);
		m_metadata.insert(wideKey, GgufValue());

		auto it = m_metadata.find(wideKey);
		if (it == m_metadata.end() || !readValue(cursor, it->second))
		{
			log::error << L"Malformed GGUF file; unreadable metadata value for \"" << wideKey << L"\"." << Endl;
			destroy();
			return false;
		}
	}

	m_tensors.resize((size_t)tensorCount);
	for (uint64_t i = 0; i < tensorCount; ++i)
	{
		GgufTensor& tensor = m_tensors[(size_t)i];

		std::string name;
		uint32_t dimCount;
		if (!cursor.readString(name) || !cursor.read(dimCount))
		{
			log::error << L"Malformed GGUF file; truncated tensor entry " << i << L"." << Endl;
			destroy();
			return false;
		}

		if (dimCount == 0 || dimCount > c_maximumDimensions)
		{
			log::error << L"Malformed GGUF file; tensor \"" << mbstows(name) << L"\" has " << dimCount << L" dimensions." << Endl;
			destroy();
			return false;
		}

		tensor.name = mbstows(name);
		tensor.dimCount = dimCount;

		for (uint32_t j = 0; j < dimCount; ++j)
		{
			if (!cursor.read(tensor.dims[j]) || tensor.dims[j] == 0)
			{
				log::error << L"Malformed GGUF file; bad dimension in tensor \"" << tensor.name << L"\"." << Endl;
				destroy();
				return false;
			}
		}

		uint32_t rawType;
		if (!cursor.read(rawType) || !cursor.read(tensor.offset))
		{
			log::error << L"Malformed GGUF file; truncated tensor entry \"" << tensor.name << L"\"." << Endl;
			destroy();
			return false;
		}

		tensor.type = (GgmlType)rawType;
		m_tensorIndex.insert(tensor.name, (uint32_t)i);
	}

	// Tensor payloads start at the first alignment boundary after the header.
	uint64_t alignment = (uint64_t)getInteger(L"general.alignment", (int64_t)c_defaultAlignment);
	if (alignment == 0 || (alignment & (alignment - 1)) != 0)
		alignment = c_defaultAlignment;

	const uint64_t dataOffset = (cursor.tell() + alignment - 1) & ~(alignment - 1);
	if (dataOffset > size)
	{
		log::error << L"Malformed GGUF file; tensor data begins past end of file." << Endl;
		destroy();
		return false;
	}

	for (auto& tensor : m_tensors)
	{
		const uint32_t blockSize = getBlockSize(tensor.type);
		if (blockSize != 0 && (tensor.dims[0] % blockSize) != 0)
		{
			log::error << L"Malformed GGUF file; row length " << tensor.dims[0] << L" of tensor \"" << tensor.name << L"\" is not a multiple of the " << getTypeName(tensor.type) << L" block size." << Endl;
			destroy();
			return false;
		}

		// Unknown types have no computable size; leave them unresolved and
		// let the model report them by name if it actually needs one.
		if (blockSize == 0)
			continue;

		const uint64_t storageSize = getStorageSize(tensor.type, tensor.getElementCount());
		if (dataOffset + tensor.offset + storageSize > size)
		{
			log::error << L"Malformed GGUF file; tensor \"" << tensor.name << L"\" extends past end of file." << Endl;
			destroy();
			return false;
		}

		tensor.data = base + dataOffset + tensor.offset;
	}

	m_architecture = mbstows(getString(L"general.architecture", ""));

	log::info << L"GGUF v" << version << L", architecture \"" << m_architecture << L"\", " << (uint32_t)m_tensors.size() << L" tensors, " << (uint32_t)m_metadata.size() << L" metadata entries." << Endl;
	return true;
}

void GgufFile::destroy()
{
	m_tensors.clear();
	m_tensorIndex.clear();
	m_metadata.clear();
	m_architecture.clear();
	m_mappedFile = nullptr;
}

const GgufValue* GgufFile::findValue(const std::wstring& key) const
{
	auto it = m_metadata.find(key);
	return it != m_metadata.end() ? &it->second : nullptr;
}

int64_t GgufFile::getInteger(const std::wstring& key, int64_t defaultValue) const
{
	const GgufValue* value = findValue(key);
	if (value == nullptr || value->isArray())
		return defaultValue;
	return value->integer;
}

double GgufFile::getNumber(const std::wstring& key, double defaultValue) const
{
	const GgufValue* value = findValue(key);
	if (value == nullptr || value->isArray())
		return defaultValue;
	return value->number;
}

bool GgufFile::getBool(const std::wstring& key, bool defaultValue) const
{
	const GgufValue* value = findValue(key);
	if (value == nullptr || value->isArray())
		return defaultValue;
	return value->integer != 0;
}

std::string GgufFile::getString(const std::wstring& key, const std::string& defaultValue) const
{
	const GgufValue* value = findValue(key);
	if (value == nullptr || value->type != GgufValueType::String)
		return defaultValue;
	return value->string;
}

const GgufTensor* GgufFile::findTensor(const std::wstring& name) const
{
	auto it = m_tensorIndex.find(name);
	return it != m_tensorIndex.end() ? &m_tensors[it->second] : nullptr;
}

int64_t GgufFile::getFileSize() const
{
	return m_mappedFile ? m_mappedFile->getSize() : 0;
}

}
