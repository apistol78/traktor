#ifndef traktor_MemberEnum_H
#define traktor_MemberEnum_H

#include <map>
#include <algorithm>
#include "Core/Serialization/MemberComplex.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Enumeration base member.
 * \ingroup Core
 */
class T_DLLCLASS MemberEnumBase : public MemberComplex
{
public:
	struct Key
	{
		const wchar_t* id;
		int val;
	};

	MemberEnumBase(const std::wstring& name, const Key* keys);

	const Key* keys() const;

	virtual bool set(const std::wstring& id) const = 0;

	virtual const wchar_t* get() const = 0;

	virtual bool serialize(Serializer& s) const;

private:
	const Key* m_keys;
};

/*! \brief Enumeration member.
 * \ingroup Core
 */
template < typename EnumType >
class MemberEnum : public MemberEnumBase
{
public:
	typedef EnumType value_type;

	MemberEnum(const std::wstring& name, EnumType& en, const Key* keys)
	:	MemberEnumBase(name, keys)
	,		m_en(en)
	{
	}

	virtual bool set(const std::wstring& id) const
	{
		for (const Key* k = keys(); k->id; ++k)
		{
			if (k->id == id)
			{
				m_en = (value_type)k->val;
				return true;
			}
		}
		return false;
	}

	virtual const wchar_t* get() const
	{
		for (const Key* k = keys(); k->id; ++k)
		{
			if (k->val == m_en)
				return k->id;
		}
		return 0;
	}

private:
	EnumType& m_en;
};

}

#endif	// traktor_MemberEnum_H
