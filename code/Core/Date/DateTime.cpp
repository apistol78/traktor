#include <ctime>
#include "Core/Date/DateTime.h"
#include "Core/Heap/Ref.h"
#include "Core/Heap/New.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace
	{

const int8_t c_monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.DateTime", DateTime, Serializable)

DateTime::DateTime()
:	m_year(0)
,	m_month(0)
,	m_day(0)
,	m_hour(0)
,	m_minute(0)
,	m_second(0)
{
}

DateTime::DateTime(uint16_t year, uint8_t month, uint16_t day, uint8_t hour, uint8_t minute, uint8_t second)
:	m_year(year)
,	m_month(month)
,	m_day(day)
,	m_hour(hour)
,	m_minute(minute)
,	m_second(second)
{
	T_ASSERT (m_month >= 1 && m_month <= 12);
	T_ASSERT (m_day >= 1 && m_day <= 31);
	T_ASSERT (m_hour >= 0 && m_hour <= 23);
	T_ASSERT (m_minute >= 0 && m_minute <= 59);
	T_ASSERT (m_second >= 0 && m_second <= 59);

	for (int8_t i = 1; i < m_month; ++i)
		m_day += c_monthDays[i - 1];

	if (m_month > 2 && isLeapYear())
		++m_day;
}

DateTime* DateTime::now()
{
#if !defined(WINCE)
	// Get current time.
	time_t t;
	::time(&t);

	// Convert to local time.
#if defined(_MSC_VER)
	struct tm tms;
	::localtime_s(&tms, &t);
	struct tm* tmp = &tms;
#else
	struct tm* tmp = ::localtime(&t);
	T_ASSERT (tmp);
#endif

	Ref< DateTime > date = gc_new< DateTime >();

	date->m_year = tmp->tm_year + 1900;
	date->m_month = tmp->tm_mon + 1;
	date->m_day = tmp->tm_yday + 1;
	date->m_hour = tmp->tm_hour;
	date->m_minute = tmp->tm_min;
	date->m_second = tmp->tm_sec;

	T_ASSERT (date->getDay() == tmp->tm_mday - 1);

	return date;
#else
	return 0;
#endif
}

uint16_t DateTime::getYear() const
{
	return m_year;
}

uint8_t DateTime::getMonth() const
{
	return m_month;
}

uint8_t DateTime::getDay() const
{
	uint16_t day = m_day;

	for (uint8_t i = 1; i < m_month; ++i)
		day -= c_monthDays[i];

	if (m_month > 2 && isLeapYear())
		--day;

	T_ASSERT (day >= 1 && day <= 31);
	return int8_t(day);
}

uint8_t DateTime::getWeekDay() const
{
	return 0;
}

uint16_t DateTime::getYearDay() const
{
	return m_day;
}

bool DateTime::isLeapYear() const
{
	if (m_year % 400 == 0)
		return true;

	if (m_year % 100 == 0)
		return false;

	if (m_year % 4 == 0)
		return true;

	return false;
}

uint8_t DateTime::getHour() const
{
	return m_hour;
}

uint8_t DateTime::getMinute() const
{
	return m_minute;
}

uint8_t DateTime::getSecond() const
{
	return m_second;
}

bool DateTime::operator == (const DateTime& dt) const
{
	return (
		m_year == dt.m_year &&
		m_month == dt.m_month &&
		m_day == dt.m_day &&
		m_hour == dt.m_hour &&
		m_minute == dt.m_minute &&
		m_second == dt.m_second
	);
}

bool DateTime::operator != (const DateTime& dt) const
{
	return !(*this == dt);
}

bool DateTime::serialize(Serializer& s)
{
	s >> Member< uint16_t >(L"year", m_year);
	s >> Member< uint8_t >(L"month", m_month);
	s >> Member< uint16_t >(L"day", m_day);
	s >> Member< uint8_t >(L"hour", m_hour);
	s >> Member< uint8_t >(L"minute", m_minute);
	s >> Member< uint8_t >(L"second", m_second);
	return true;
}

}
