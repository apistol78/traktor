/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Date and time class.
 * \ingroup Core
 */
class T_DLLCLASS DateTime : public ISerializable
{
	T_RTTI_CLASS;

public:
	DateTime() = default;

	/*! Initialize DateTime object from seconds since the Epoch.
	 *
	 * \param seconds Seconds since the Epoch 1970-01-01 00:00:00 UTC.
	 */
	explicit DateTime(uint64_t seconds);

	/*! Initialize DateTime object with given date.
	 *
	 * \param year Fully qualified year.
	 * \param month Month (1 - 12).
	 * \param day Day of month (1 - 31).
	 * \param hour Hour (0 - 23).
	 * \param minute Minute (0 - 59).
	 * \param second Second (0 - 59).
	 */
	explicit DateTime(uint16_t year, uint8_t month, uint16_t day, uint8_t hour, uint8_t minute, uint8_t second);

	/*! Return current date. */
	static DateTime now();

	/*! Parse date; must conform to ISO 8601. */
	static DateTime parse(const std::wstring& str);

	/*! Return year. */
	uint16_t getYear() const;

	/*! Return month (1 - 12). */
	uint8_t getMonth() const;

	/*! Return day of month (1 - 31). */
	uint8_t getDay() const;

	/*! Return week day (0 = Sunday). */
	uint8_t getWeekDay() const;

	/*! Return day of year (1 - 366). */
	uint16_t getYearDay() const;

	/*! Return true if current year is a leap year. */
	bool isLeapYear() const;

	/*! Return hour (0 - 23). */
	uint8_t getHour() const;

	/*! Return minute (0 - 59). */
	uint8_t getMinute() const;

	/*! Return second (0 - 59). */
	uint8_t getSecond() const;

	/*! Return seconds since the Epoch. */
	uint64_t getSecondsSinceEpoch() const;

	/*! Format as string.
	 *
	 * See strftime for further information.
	 *
	 * \param fmt Format.
	 */
	std::wstring format(const std::wstring& fmt) const;

	/*! Implicit convert to epoch. */
	operator uint64_t () const;

	/*! Serialize object. */
	virtual void serialize(ISerializer& s) override final;

private:
	uint64_t m_epoch = 0;
};

}

