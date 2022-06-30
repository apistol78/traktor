#pragma once

#include <functional>
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Enumeration base member.
 * \ingroup Core
 */
class T_DLLCLASS MemberEnumBase : public MemberComplex
{
public:
	MemberEnumBase(const wchar_t* const name);

	virtual bool set(const std::wstring& id) const = 0;

	virtual const wchar_t* const get() const = 0;

	virtual void enumerate(const std::function< void(const wchar_t*) >& fn) const = 0;

	virtual void serialize(ISerializer& s) const override final;
};

/*! Enumeration member.
 * \ingroup Core
 */
template < typename EnumType >
class MemberEnum : public MemberEnumBase
{
public:
	typedef EnumType value_type;

	struct Key
	{
		const wchar_t* id;
		EnumType val;
	};

	explicit MemberEnum(const wchar_t* const name, EnumType& en, const Key* keys)
	:	MemberEnumBase(name)
	,	m_keys(keys)
	,	m_en(en)
	{
	}

	virtual bool set(const std::wstring& id) const override final
	{
		for (const Key* k = m_keys; k->id; ++k)
		{
			if (k->id == id)
			{
				m_en = (value_type)k->val;
				return true;
			}
		}
		return false;
	}

	virtual const wchar_t* const get() const override final
	{
		for (const Key* k = m_keys; k->id; ++k)
		{
			if (k->val == m_en)
				return k->id;
		}
		return nullptr;
	}

	virtual void enumerate(const std::function< void(const wchar_t*) >& fn) const override final
	{
		for (const Key* k = m_keys; k->id; ++k)
			fn(k->id);		
	}

	const Key* keys() const { return m_keys; }

private:
	const Key* m_keys;
	EnumType& m_en;
};

/*! Enumeration member.
 * \ingroup Core
 */
template < typename EnumType, typename ValueType = uint32_t >
class MemberEnumByValue : public MemberComplex
{
public:
	explicit MemberEnumByValue(const wchar_t* const name, EnumType& en)
	:	MemberComplex(name, false)
	,	m_en(en)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		ValueType value = static_cast< ValueType >(m_en);
		s >> Member< ValueType >(getName(), value);
		m_en = static_cast< EnumType >(value);
	}

private:
	EnumType& m_en;
};

}

