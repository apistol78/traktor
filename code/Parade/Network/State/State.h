#ifndef traktor_parade_State_H
#define traktor_parade_State_H

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace parade
	{

class IValue;

class T_DLLCLASS State : public Object
{
	T_RTTI_CLASS;

public:
	State();

	State(const RefArray< const IValue > values);

	const RefArray< const IValue >& getValues() const { return m_values; }

	/*! \group Pack values */
	// \{

	void packBegin();

	void pack(const IValue* value);

	template < typename ValueType >
	void pack(typename ValueType::value_argument_t value) { pack(new ValueType(value)); }

	// \}

	/*! \group Unpack values. */
	// \{

	void unpackBegin();

	const IValue* unpack();

	template < typename ValueType >
	typename ValueType::value_return_t unpack() { return *checked_type_cast< const ValueType*, false >(unpack()); }

	// \}

private:
	RefArray< const IValue > m_values;
	uint32_t m_index;
};

	}
}

#endif	// traktor_parade_State_H
