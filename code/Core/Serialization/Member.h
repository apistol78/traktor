#ifndef traktor_Member_H
#define traktor_Member_H

#include <string>

namespace traktor
{

/*! \ingroup Core */
//@{

class Type;

template < typename T >
class Member
{
public:
	typedef T value_type;

	Member< T >(const std::wstring& name, value_type& ref, const Type* type = 0)
	:	m_name(name)
	,	m_ref(ref)
	,	m_type(type)
	{
	}
	
	const std::wstring& getName() const
	{
		return m_name;
	}

	virtual const Type* getType() const
	{
		return m_type;
	}
	
	operator value_type& () const
	{
		return m_ref;
	}
	
	value_type& operator * () const
	{
		return m_ref;
	}
	
	value_type* operator -> () const
	{
		return &m_ref;
	}
	
	void operator = (const value_type& value) const
	{
		m_ref = value;
	}
	
private:
	std::wstring m_name;
	value_type& m_ref;
	const Type* m_type;
};

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
	
	const std::wstring& getName() const
	{
		return m_name;
	}

	bool isMultiLine() const
	{
		return m_multiLine;
	}

	operator value_type& () const
	{
		return m_ref;
	}
	
	value_type& operator * () const
	{
		return m_ref;
	}
	
	value_type* operator -> () const
	{
		return &m_ref;
	}
	
	void operator = (const value_type& value) const
	{
		m_ref = value;
	}
	
private:
	std::wstring m_name;
	value_type& m_ref;
	bool m_multiLine;
};

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
	
	const std::wstring& getName() const
	{
		return m_name;
	}

	void* getBlob() const
	{
		return m_blob;
	}

	uint32_t getBlobSize() const
	{
		return m_blobSize;
	}

	void setBlobSize(uint32_t blobSize) const
	{
		m_blobSize = blobSize;
	}

private:
	std::wstring m_name;
	void* m_blob;
	uint32_t& m_blobSize;
};

//@}

}

#endif	// traktor_Member_H
