/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Save_H
#define traktor_Save_H

namespace traktor
{

/*! \brief Scoped value.
 * \ingroup Core
 *
 * Automatic restoration of variable values when getting out of scope.
 * Useful when a function has several return points.
 * \example
 * {
 *    Save< int > saveFoo(g_foo);
 *    g_foo = 2;
 *    // after function finished g_foo will be restored to it's original value.
 * }
 */
template < typename T >
class Save
{
public:
	Save(T& var)
	:	m_ref(var)
	,	m_val(var)
	{
	}

	Save(T& var, T value)
	:	m_ref(var)
	,	m_val(var)
	{
		m_ref = value;
	}

	~Save()
	{
		m_ref = m_val;
	}
	
private:
	T& m_ref;
	T m_val;
};

}

#endif	// traktor_Save_H
