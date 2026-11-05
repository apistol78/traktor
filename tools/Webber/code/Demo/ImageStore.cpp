/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <cwctype>
#include "Core/Guid.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Demo/ImageStore.h"

namespace traktor::demo
{
	namespace
	{

const wchar_t* c_uriPrefix = L"/images/";
const size_t c_nameLength = 32;

bool startsWithBytes(const AlignedVector< uint8_t >& data, size_t offset, const char* bytes, size_t length)
{
	return data.size() >= offset + length && std::memcmp(data.c_ptr() + offset, bytes, length) == 0;
}

/*! File extension from image content; empty if not a supported image. */
std::wstring imageExtension(const AlignedVector< uint8_t >& data)
{
	if (startsWithBytes(data, 0, "\x89PNG\r\n\x1a\n", 8))
		return L"png";
	else if (startsWithBytes(data, 0, "\xff\xd8\xff", 3))
		return L"jpg";
	else if (startsWithBytes(data, 0, "GIF87a", 6) || startsWithBytes(data, 0, "GIF89a", 6))
		return L"gif";
	else if (startsWithBytes(data, 0, "RIFF", 4) && startsWithBytes(data, 8, "WEBP", 4))
		return L"webp";
	else
		return L"";
}

/*! Stored names are 32 lower case hex digits and a supported extension, nothing else is ever opened. */
bool isValidName(const std::wstring& name)
{
	const size_t p = name.find(L'.');
	if (p != c_nameLength || name.substr(0, p).find_first_not_of(L"0123456789abcdef") != name.npos)
		return false;

	const std::wstring extension = name.substr(p + 1);
	return extension == L"png" || extension == L"jpg" || extension == L"gif" || extension == L"webp";
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.demo.ImageStore", ImageStore, Object)

ImageStore::ImageStore(const Path& directory)
:	m_directory(directory)
{
}

std::wstring ImageStore::store(const AlignedVector< uint8_t >& data)
{
	const std::wstring extension = imageExtension(data);
	if (extension.empty())
		return L"";

	std::wstring name;
	for (auto ch : Guid::create().format())
	{
		if (std::iswxdigit(ch))
			name += (wchar_t)std::towlower(ch);
	}
	name += L"." + extension;

	FileSystem::getInstance().makeAllDirectories(m_directory);
	Ref< IStream > file = FileSystem::getInstance().open(m_directory + Path(name), File::FmWrite);
	if (!file)
	{
		log::error << L"Unable to save image \"" << name << L"\"." << Endl;
		return L"";
	}

	const bool written = file->write(data.c_ptr(), (int64_t)data.size()) == (int64_t)data.size();
	file->close();
	if (!written)
	{
		FileSystem::getInstance().remove(m_directory + Path(name));
		log::error << L"Unable to save image \"" << name << L"\"." << Endl;
		return L"";
	}

	return std::wstring(c_uriPrefix + 1) + name;
}

Ref< IStream > ImageStore::open(const std::wstring& uri) const
{
	if (!startsWith(uri, c_uriPrefix))
		return nullptr;

	const std::wstring name = uri.substr(std::wcslen(c_uriPrefix));
	if (!isValidName(name))
		return nullptr;

	return FileSystem::getInstance().open(m_directory + Path(name), File::FmRead);
}

}
