#ifndef traktor_flash_Array_H
#define traktor_flash_Array_H

#include <algorithm>
#include "Core/Containers/AlignedVector.h"
#include "Flash/Action/ActionObjectRelay.h"
#include "Flash/Action/ActionValueArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Array of ActionValue objects.
 * \ingroup Flash
 */
class T_DLLCLASS Array : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Array(uint32_t capacity);

	Array(const ActionValueArray& values);

	Ref< Array > concat() const;

	Ref< Array > concat(const ActionValueArray& values) const;

	std::string join(const std::string& delimiter) const;

	void push(const ActionValue& value);

	ActionValue pop();

	void reverse();

	ActionValue shift();

	uint32_t unshift(const ActionValueArray& values);

	Ref< Array > slice(int32_t startIndex, int32_t endIndex) const;

	Ref< Array > splice(int32_t startIndex, uint32_t deleteCount, const ActionValueArray& values, int32_t offset);

	uint32_t length() const;

	int32_t indexOf(const ActionValue& value) const;

	void removeAt(int32_t index);

	void removeAll();

	template < typename PredicateType >
	void sort(const PredicateType& predicate)
	{
		std::sort(m_values.begin(), m_values.end(), predicate);
	}

	const AlignedVector< ActionValue >& getValues() const { return m_values; }

	ActionValue& operator [] (int32_t index) { return m_values[index]; }

	const ActionValue& operator [] (int32_t index) const { return m_values[index]; }

protected:
	virtual void trace(visitor_t visitor) const;

	virtual void dereference();

private:
	AlignedVector< ActionValue > m_values;
};

	}
}

#endif	// traktor_flash_Array_H
