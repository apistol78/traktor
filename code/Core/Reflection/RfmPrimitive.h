#ifndef traktor_RfmPrimitive_H
#define traktor_RfmPrimitive_H

#include "Core/Reflection/ReflectionMember.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \brief Primitive member reflection.
 * \ingroup Core
 */
template < typename PrimitiveType >
class RfmPrimitive : public ReflectionMember
{
	T_RTTI_CLASS;

public:
	typedef PrimitiveType type_t;

	RfmPrimitive(const wchar_t* name, const type_t& value)
	:	ReflectionMember(name)
	,	m_value(value)
	{
	}

	void set(const type_t& value) { m_value = value; }

	const type_t& get() const { return m_value; }

	virtual bool replace(const ReflectionMember* source)
	{
		typedef RfmPrimitive< PrimitiveType > type_t;
		if (const type_t* sourceType = dynamic_type_cast< const type_t* >(source))
		{
			m_value = sourceType->m_value;
			return true;
		}
		else
			return false;
	}

private:
	type_t T_ALIGN16 m_value;
};

T_IMPLEMENT_RTTI_TEMPLATE_CLASS(RfmPrimitive< T >, typename T, Object)

}

#endif	// traktor_RfmPrimitive_H
