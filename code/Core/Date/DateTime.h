#ifndef traktor_DateTime_H
#define traktor_DateTime_H

#include "Core/Serialization/Serializable.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Date and time class.
 * \ingroup Core
 */
class T_DLLCLASS DateTime : public Serializable
{
	T_RTTI_CLASS(DateTime)

public:
	DateTime();

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
	static Ref< DateTime > now();

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

	/*! \brief Compare equal operator. */
	bool operator == (const DateTime& dt) const;

	/*! \brief Compare not-equal operator. */
	bool operator != (const DateTime& dt) const;

	virtual bool serialize(Serializer& s);

private:
	uint16_t m_year;
	uint8_t m_month;
	uint16_t m_day;
	uint8_t m_hour;
	uint8_t m_minute;
	uint8_t m_second;
};

}

#endif	// traktor_DateTime_H
