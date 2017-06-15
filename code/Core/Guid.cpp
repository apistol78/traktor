/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cctype>
#include <cstring>
#if defined(__APPLE__)
#	include <mach/mach_time.h>
#	include "Core/Math/Random.h"
#elif defined(__LINUX__)
#	include <uuid/uuid.h>
#endif
#include "Core/Platform.h"
#include "Core/Guid.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{

Guid::Guid()
:	m_valid(false)
{
	std::memset(m_data, 0, 16);
}

Guid::Guid(const std::wstring& s)
:	m_valid(false)
{
	create(s);
}

Guid::Guid(const uint8_t data[16])
:	m_valid(true)
{
	std::memcpy(m_data, data, 16);
}

bool Guid::create(const std::wstring& s)
{
	// {00000000-0000-0000-0000-000000000000}
	// 01234567890123456789012345678901234567
	// 0         1         2         3

	m_valid = false;

	if (s.length() < 38)
		return false;

	if (s[0] != L'{' || s[9] != L'-' || s[14] != L'-' || s[19] != L'-' || s[24] != L'-' || s[37] != L'}')
		return false;

	for (int i = 0; i < 16; ++i)
	{
		int o = i * 2 + 1, tmp = 0;

		if (o >= 9)
			o++;
		if (o >= 14)
			o++;
		if (o >= 19)
			o++;
		if (o >= 24)
			o++;

		wchar_t hn = std::tolower(s[o]);
		wchar_t ln = std::tolower(s[o + 1]);

		if (hn >= L'0' && hn <= L'9')
			tmp = (hn - L'0') << 4;
		else if (hn >= L'a' && hn <= L'f')
			tmp = (hn - L'a' + 10) << 4;
		else
			return false;

		if (ln >= L'0' && ln <= L'9')
			tmp |= ln - L'0';
		else if (ln >= L'a' && ln <= L'f')
			tmp |= ln - L'a' + 10;
		else
			return false;

		m_data[i] = static_cast< uint8_t >(tmp);
	}

	m_valid = true;
	return true;
}

Guid Guid::create()
{
	Guid guid;

#if defined(_WIN32) && !defined(_XBOX) && !defined(_XBOX_ONE)

	GUID tmp;
	CoCreateGuid(&tmp);
	std::memcpy(guid.m_data, &tmp, 16);
	guid.m_valid = true;

#elif defined(__APPLE__)

	uint64_t cputick = mach_absolute_time();
	std::memcpy(guid.m_data, &cputick, sizeof(cputick));

	static Random s_rnd;
	for (int i = 8; i < 16; ++i)
		guid.m_data[i] = uint8_t(s_rnd.nextDouble() * 255);

	guid.m_valid = true;

#elif defined(__LINUX__)

	uuid_t id;
	uuid_generate(id);
	std::memcpy(guid.m_data, id, 16);
	guid.m_valid = true;

#endif

	return guid;
}

std::wstring Guid::format() const
{
	const wchar_t hex[] = { L"0123456789ABCDEF" };

	StringOutputStream ss;
	ss.put(L'{');

	for (int i = 0; i < 16; ++i)
	{
		if (i == 4 || i == 6 || i == 8 || i == 10)
			ss.put(L'-');
		ss.put(hex[m_data[i] >> 4]);
		ss.put(hex[m_data[i] & 0x0f]);
	}

	ss.put(L'}');

	return ss.str();
}

bool Guid::isValid() const
{
	return m_valid;
}

bool Guid::isNull() const
{
	for (int i = 0; i < sizeof_array(m_data); ++i)
	{
		if (m_data[i] != 0)
			return false;
	}
	return true;
}

bool Guid::isNotNull() const
{
	return isValid() && !isNull();
}

Guid Guid::permutate(uint32_t iterations) const
{
	uint8_t d[16];
	std::memcpy(d, m_data, 16);
	reinterpret_cast< uint64_t& >(d[8]) += iterations;
	return Guid(d);
}

Guid Guid::permutate(const Guid& seed) const
{
	uint8_t d[16];
	for (int i = 0; i < 16; ++i)
		d[i] = m_data[i] ^ seed[i];
	return Guid(d);
}

Guid::operator const uint8_t* () const
{
	return m_data;
}

bool Guid::operator == (const Guid& r) const
{
	return std::memcmp(m_data, r.m_data, 16) == 0;
}

bool Guid::operator != (const Guid& r) const
{
	return std::memcmp(m_data, r.m_data, 16) != 0;
}

bool Guid::operator < (const Guid& r) const
{
	return std::memcmp(m_data, r.m_data, 16) < 0;
}

bool Guid::operator > (const Guid& r) const
{
	return std::memcmp(m_data, r.m_data, 16) > 0;
}

}
