#ifndef traktor_DateTime_H
#define traktor_DateTime_H

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

/*! \brief Date and time class.
 * \ingroup Core
 */
class T_DLLCLASS DateTime : public ISerializable
{
	T_RTTI_CLASS;

public:
	DateTime();

	/*! \brief Initialize DateTime object from seconds since the Epoch.
	 *
	 * \param seconds Seconds since the Epoch 1970-01-01 00:00:00 UTC.
	 */
	DateTime(uint64_t seconds);

	/*! \brief Initialize DateTime object with given date.
	 *
	 * \param year Fully qualified year.
	 * \param month Month (1 - 12).
	 * \param day Day of month (1 - 31).
	 * \param hour Hour (0 - 23).
	 * \param minute Minute (0 - 59).
	 * \param second Second (0 - 59).
	 */
	DateTime(uint16_t year, uint8_t month, uint16_t day, uint8_t hour, uint8_t minute, uint8_t second);

	/*! \brief Return current date. */
	static DateTime now();

	/*! \brief Parse date; must conform to ISO 8601. */
	static DateTime parse(const std::wstring& str);

	/*! \brief Return year. */
	uint16_t getYear() const;

	/*! \brief Return month (1 - 12). */
	uint8_t getMonth() const;

	/*! \brief Return day of month (1 - 31). */
	uint8_t getDay() const;

	/*! \brief Return week day (0 = Sunday). */
	uint8_t getWeekDay() const;

	/*! \brief Return day of year (1 - 366). */
	uint16_t getYearDay() const;

	/*! \brief Return true if current year is a leap year. */
	bool isLeapYear() const;

	/*! \brief Return hour (0 - 23). */
	uint8_t getHour() const;

	/*! \brief Return minute (0 - 59). */
	uint8_t getMinute() const;

	/*! \brief Return second (0 - 59). */
	uint8_t getSecond() const;

	/*! \brief Return seconds since the Epoch. */
	uint64_t getSecondsSinceEpoch() const;

	/*! \brief Format as string.
	 *
	 * See strftime for further information.
	 *
	 * \param fmt Format.
	 */
	std::wstring format(const std::wstring& fmt) const;

	/*! \brief Implicit convert to epoch. */
	operator uint64_t () const;

	/*! \brief Serialize object. */
	virtual void serialize(ISerializer& s);

private:
	uint64_t m_epoch;
};

}

#endif	// traktor_DateTime_H
