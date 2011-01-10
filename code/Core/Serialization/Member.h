#ifndef traktor_Member_H
#define traktor_Member_H

#include <limits>
#include <string>
#include "Core/Config.h"

namespace traktor
{

/*! \ingroup Core */
//@{

class Attribute;

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

	Member(const std::wstring& name, value_type& ref)
	:	m_name(name)
	,	m_ref(ref)
	,	m_attributes(0)
	{
	}

	Member(const std::wstring& name, value_type& ref, const Attribute& attributes)
	:	m_name(name)
	,	m_ref(ref)
	,	m_attributes(&attributes)
	{
	}

	virtual ~Member() {}
	
	/*! \brief Get member name.
	 *
	 * \return Member name.
	 */
	const std::wstring& getName() const { return m_name; }

	/*! \brief Get member attributes.
	 *
	 * Get member attributes if applicable.
	 *
	 * \return Member attributes.
	 */
	virtual const Attribute* getAttributes() const { return m_attributes; }
	
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
	const Attribute* m_attributes;
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
