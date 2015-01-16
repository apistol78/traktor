#ifndef traktor_net_State_H
#define traktor_net_State_H

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class IValue;

class T_DLLCLASS State : public Object
{
	T_RTTI_CLASS;

public:
	State();

	State(const RefArray< const IValue >& values);

	const RefArray< const IValue >& getValues() const { return m_values; }

	const IValue* getValue(uint32_t index) const { return m_values[index]; }

	template < typename ValueType >
	typename ValueType::value_return_t getValue(uint32_t index) const { return *checked_type_cast< const ValueType*, false >(getValue(index)); }

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

#endif	// traktor_net_State_H
