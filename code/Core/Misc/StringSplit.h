/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_StringSplit_H
#define traktor_StringSplit_H

#include "Core/Config.h"

namespace traktor
{

/*! \brief String split iterator.
 * \ingroup Core
 *
 * More efficient implementation of splitting string
 * by delimiters.
 * Implemented with iterator pattern instead of
 * allocating an output vector.
 */
template < typename StringType >
class StringSplit
{
public:
	typedef StringType string_type;
	typedef typename StringType::size_type size_type;

	class const_iterator
	{
	public:
		typedef std::forward_iterator_tag iterator_category;
		typedef string_type value_type;

		string_type operator * () const
		{
			if (m_st != string_type::npos)
				return m_this->m_str.substr(m_st, m_en - m_st);
			else
				return string_type();
		}

		const_iterator operator ++ ()
		{
			if (m_en != string_type::npos)
			{
				m_st = m_en + 1;
				m_en = m_this->m_str.find_first_of(m_this->m_delimiters, m_st);
			}
			else
			{
				m_st =
				m_en = string_type::npos;
			}
			return *this;
		}

		const_iterator operator ++ (int)
		{
			size_type st = m_st, en = m_en;
			if (m_en != string_type::npos)
			{
				m_st = m_en + 1;
				m_en = m_this->m_str.find_first_of(m_this->m_delimiters, m_st);
			}
			else
			{
				m_st =
				m_en = string_type::npos;
			}
			return const_iterator(m_this, st, en);
		}

		bool operator == (const const_iterator& it) const
		{
			T_ASSERT (m_this == it.m_this);
			return m_st == it.m_st && m_en == it.m_en;
		}

		bool operator != (const const_iterator& it) const
		{
			T_ASSERT (m_this == it.m_this);
			return m_st != it.m_st || m_en != it.m_en;
		}

	private:
		friend class StringSplit;

		const StringSplit* m_this;
		size_type m_st;
		size_type m_en;

		const_iterator(const StringSplit* this_, size_type st, size_type en)
		:	m_this(this_)
		,	m_st(st)
		,	m_en(en)
		{
		}
	};

	StringSplit(const string_type& str, const string_type& delimiters)
	:	m_str(str)
	,	m_delimiters(delimiters)
	{
	}

	const_iterator begin() const
	{
		size_type st = m_str.find_first_not_of(m_delimiters);
		size_type en = m_str.find_first_of(m_delimiters, st);
		return const_iterator(this, st, en);
	}

	const_iterator end() const
	{
		return const_iterator(
			this,
			string_type::npos,
			string_type::npos
		);
	}

private:
	string_type m_str;
	string_type m_delimiters;
};

}

#endif	// traktor_StringSplit_H
