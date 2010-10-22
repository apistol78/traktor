#ifndef traktor_Member_H
#define traktor_Member_H

#include <limits>
#include <string>
#include "Core/Config.h"

namespace traktor
{

/*! \ingroup Core */
//@{

class TypeInfo;

/*! \brief Member serialization.
 * \ingroup Core
 *
 * Member class is a named descriptor of a member reference.
 * Serializer access named member through this object, both
 * when reading and writing to the members.
 */
template < typename T >
class Member
{
public:
	typedef T value_type;

	Member(const std::wstring& name, value_type& ref, const TypeInfo* type = 0)
	:	m_name(name)
	,	m_ref(ref)
	,	m_type(type)
	{
	}

	virtual ~Member() {}
	
	/*! \brief Get member name.
	 *
	 * \return Member name.
	 */
	const std::wstring& getName() const { return m_name; }

	/*! \brief Get member type.
	 *
	 * Get member type if applicable.
	 *
	 * \return Member type.
	 */
	virtual const TypeInfo* getType() const { return m_type; }
	
	/*! \brief Get member reference.
	 *
	 * \return Member reference.
	 */
	operator value_type& () const { return m_ref; }
	
	/*! \brief Dereference member.
	 *
	 * \return Member reference.
	 */
	value_type& operator * () const { return m_ref; }
	
	/*! \brief Dereference member.
	 *
	 * \return Member pointer.
	 */
	value_type* operator -> () const { return &m_ref; }
	
	/*! \brief Assign value to member.
	 *
	 * \param value New member value.
	 */
	void operator = (const value_type& value) const { m_ref = value; }
	
private:
	std::wstring m_name;
	value_type& m_ref;
	const TypeInfo* m_type;
};

/*! \brief Float member serialization.
 * \ingroup Core
 *
 * Specialized Member-class for float members as we want
 * additional information about the data representation.
 */
template < >
class Member < float >
{
public:
	typedef float value_type;

	Member(
		const std::wstring& name,
		value_type& ref,
		value_type limitMin = -std::numeric_limits< value_type >::max(),
		value_type limitMax = std::numeric_limits< value_type >::max()
	)
	:	m_name(name)
	,	m_ref(ref)
	,	m_limitMin(limitMin)
	,	m_limitMax(limitMax)
	{
	}
	
	/*! \brief Get member name.
	 *
	 * \return Member name.
	 */
	const std::wstring& getName() const { return m_name; }

	/*! \brief Get minimum limit.
	 *
	 * \return Minimum limit.
	 */
	value_type getLimitMin() const { return m_limitMin; }

	/*! \brief Get maximum limit.
	 *
	 * \return Maximum limit.
	 */
	value_type getLimitMax() const { return m_limitMax; }

	/*! \brief Get member reference.
	 *
	 * \return Member reference.
	 */
	operator value_type& () const { return m_ref; }
	
	/*! \brief Dereference member.
	 *
	 * \return Member reference.
	 */
	value_type& operator * () const { return m_ref; }
	
	/*! \brief Dereference member.
	 *
	 * \return Member pointer.
	 */
	value_type* operator -> () const { return &m_ref; }
	
	/*! \brief Assign value to member.
	 *
	 * \param value New member value.
	 */
	void operator = (const value_type& value) const
	{
		T_ASSERT (value >= m_limitMin);
		T_ASSERT (value <= m_limitMax);
		m_ref = value;
	}
	
private:
	std::wstring m_name;
	value_type& m_ref;
	value_type m_limitMin;
	value_type m_limitMax;
};

/*! \brief String member serialization.
 * \ingroup Core
 *
 * Specialized Member-class for string members as we want
 * additional information about the data representation.
 */
template < >
class Member < std::wstring >
{
public:
	typedef std::wstring value_type;

	Member(const std::wstring& name, value_type& ref, bool multiLine = false)
	:	m_name(name)
	,	m_ref(ref)
	,	m_multiLine(multiLine)
	{
	}
	
	/*! \brief Get member name.
	 *
	 * \return Member name.
	 */
	const std::wstring& getName() const { return m_name; }

	/*! \brief Return true if string have multiple lines.
	 *
	 * \return True if multiple lines.
	 */
	bool isMultiLine() const { return m_multiLine; }

	/*! \brief Get member reference.
	 *
	 * \return Member reference.
	 */
	operator value_type& () const { return m_ref; }
	
	/*! \brief Dereference member.
	 *
	 * \return Member reference.
	 */
	value_type& operator * () const { return m_ref; }
	
	/*! \brief Dereference member.
	 *
	 * \return Member pointer.
	 */
	value_type* operator -> () const { return &m_ref; }
	
	/*! \brief Assign value to member.
	 *
	 * \param value New member value.
	 */
	void operator = (const value_type& value) const { m_ref = value; }
	
private:
	std::wstring m_name;
	value_type& m_ref;
	bool m_multiLine;
};

/*! \brief Any-data member serialization.
 * \ingroup Core
 *
 * Specialized Member-class for data members as we want
 * additional information about the representation.
 */
template < >
class Member< void* >
{
public:
	Member< void* >(const std::wstring& name, void* blob, uint32_t& blobSize)
	:	m_name(name)
	,	m_blob(blob)
	,	m_blobSize(blobSize)
	{
	}
	
	/*! \brief Get member name.
	 *
	 * \return Member name.
	 */
	const std::wstring& getName() const { return m_name; }

	/*! \brief Get pointer to binary blob.
	 *
	 * \return Pointer to binary blob.
	 */
	void* getBlob() const { return m_blob; }

	/*! \brief Get size of binary blob.
	 *
	 * \return Size of binary blob.
	 */
	uint32_t getBlobSize() const { return m_blobSize; }

	/*! \brief Set size of binary blob.
	 *
	 * \param blobSize Size of binary blob.
	 */
	void setBlobSize(uint32_t blobSize) const { m_blobSize = blobSize; }

private:
	std::wstring m_name;
	void* m_blob;
	uint32_t& m_blobSize;
};

//@}

}

#endif	// traktor_Member_H
