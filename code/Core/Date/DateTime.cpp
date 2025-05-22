/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Date/DateTime.h"

#include "Core/Serialization/ISerializer.h"

#include <cstring>
#include <ctime>

namespace traktor
{
namespace
{

void getLocalTime(time_t t, struct tm* T)
{
#if defined(_MSC_VER)
	::localtime_s(T, &t);
#else
	struct tm* tmp = ::localtime(&t);
	T_ASSERT(tmp);
	std::memcpy(T, tmp, sizeof(struct tm));
#endif
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.DateTime", 0, DateTime, ISerializable)

DateTime::DateTime(uint64_t seconds)
	: m_epoch(seconds)
{
}

DateTime::DateTime(uint16_t year, uint8_t month, uint16_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
	T_ASSERT(month >= 1 && month <= 12);
	T_ASSERT(day >= 1 && day <= 31);
	T_ASSERT(hour <= 23);
	T_ASSERT(minute <= 59);
	T_ASSERT(second <= 59);

	struct tm t;
	t.tm_sec = second;
	t.tm_min = minute;
	t.tm_hour = hour;
	t.tm_mday = day - 1;
	t.tm_mon = month - 1;
	t.tm_year = year - 1900;
	t.tm_wday = 0;
	t.tm_yday = 0;
	t.tm_isdst = 0;

#if defined(_MSC_VER)
	m_epoch = ::_mkgmtime(&t);
#else
	m_epoch = ::timegm(&t);
#endif
}

DateTime DateTime::now()
{
	time_t t;
	::time(&t);
	return DateTime(uint64_t(t));
}

DateTime DateTime::parse(const std::wstring& str)
{
	T_BREAKPOINT;
	return DateTime();
}

uint16_t DateTime::getYear() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_year + 1900;
}

uint8_t DateTime::getMonth() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_mon + 1;
}

uint8_t DateTime::getDay() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_mday;
}

uint8_t DateTime::getWeekDay() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_wday;
}

uint16_t DateTime::getYearDay() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_yday;
}

bool DateTime::isLeapYear() const
{
	int32_t year = getYear();

	if (year % 400 == 0)
		return true;

	if (year % 100 == 0)
		return false;

	if (year % 4 == 0)
		return true;

	return false;
}

uint8_t DateTime::getHour() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_hour;
}

uint8_t DateTime::getMinute() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_min;
}

uint8_t DateTime::getSecond() const
{
	struct tm T;
	getLocalTime(m_epoch, &T);
	return T.tm_sec;
}

uint64_t DateTime::getSecondsSinceEpoch() const
{
	return m_epoch;
}

std::wstring DateTime::format(const std::wstring& fmt) const
{
	struct tm T;
	getLocalTime(m_epoch, &T);

	wchar_t buf[256];
	wcsftime(buf, sizeof_array(buf), fmt.c_str(), &T);

	return buf;
}

DateTime::operator uint64_t() const
{
	return m_epoch;
}

void DateTime::serialize(ISerializer& s)
{
	s >> Member< uint64_t >(L"epoch", m_epoch);
}

}
