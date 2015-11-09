#ifndef traktor_ComRef_H
#define traktor_ComRef_H

#include <vector>

namespace traktor
{

/*! \brief ActiveX/COM object reference container.
 * \ingroup Core
 *
 * Handles automatic reference counting of IUnknown objects.
 * Use this as it automatically handles proper reference counting
 * calls in case of exceptions.
 */
template < typename T >
struct ComRef
{
public:
	ComRef()
	:	m_ptr(0)
	{
	}

	ComRef(T* ptr)
	:	m_ptr(ptr)
	{
		if (m_ptr != 0)
			m_ptr->AddRef();
	}
	
	ComRef(const ComRef< T >& src)
	:	m_ptr(src.m_ptr)
	{
		if (m_ptr != 0)
			m_ptr->AddRef();
	}
	
	virtual ~ComRef()
	{
		if (m_ptr != 0)
			m_ptr->Release();
	}

	inline ComRef< T >& operator = (T* ptr)
	{
		if (ptr != 0)
			ptr->AddRef();
		if (m_ptr != 0)
			m_ptr->Release();
		m_ptr = ptr;
		return *this;
	}
	
	inline ComRef< T >& operator = (const ComRef< T >& src)
	{
		if (src.m_ptr != 0)
			src.m_ptr->AddRef();
		if (m_ptr != 0)
			m_ptr->Release();
		m_ptr = src.m_ptr;
		return *this;
	}

	inline T* const & get()
	{
		return m_ptr;
	}

	inline T*& getAssign()
	{
		release();
		return m_ptr;
	}

	inline void release()
	{
		if (m_ptr != 0)
		{
			m_ptr->Release();
			m_ptr = 0;
		}
	}

	inline operator T* () const
	{
		return m_ptr;
	}

	inline T* operator -> () const
	{
		return m_ptr;
	}
	
private:
	T* m_ptr;
};

/*! \brief ActiveX/COM object array reference container.
 * \ingroup Core
 *
 * Handles automatic reference counting of IUnknown objects.
 * Use this as it automatically handles proper reference counting
 * calls in case of exceptions.
 */
template < typename T >
struct ComRefArray
{
public:
	typedef std::vector< T* > container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;

	struct element
	{
		container_type& m_array;
		size_type m_index;

		element(container_type& array, size_type index)
		:	m_array(array)
		,	m_index(index)
		{
		}

		operator T* ()
		{
			return m_array[m_index];
		}

		element& operator = (T* ptr)
		{
			if (ptr)
				ptr->AddRef();

			if (m_array[m_index])
				m_array[m_index]->Release();

			m_array[m_index] = ptr;

			return *this;
		}
	};

	ComRefArray()
	{
	}

	ComRefArray(size_t size)
	:	m_array(size)
	{
	}
	
	ComRefArray(T** array, size_type size)
	:	m_array(size)
	{
		for (size_type i = 0; i < size; ++i)
		{
			if ((m_array[i] = array[i]) != 0)
				m_array[i]->AddRef();
		}
	}

	ComRefArray(const ComRefArray< T >& src)
	:	m_array(src.m_array)
	{
		for (typename container_type::iterator i = m_array.begin(); i != m_array.end(); ++i)
		{
			if (*i)
				(*i)->AddRef();
		}
	}
	
	virtual ~ComRefArray()
	{
		for (typename container_type::iterator i = m_array.begin(); i != m_array.end(); ++i)
		{
			if (*i)
				(*i)->Release();
		}
	}

	T** base()
	{
		return m_array.empty() ? 0 : &m_array[0];
	}

	iterator begin()
	{
		return m_array.begin();
	}

	iterator end()
	{
		return m_array.end();
	}

	const_iterator begin() const
	{
		return m_array.begin();
	}

	const_iterator end() const
	{
		return m_array.end();
	}

	T* front()
	{
		return m_array.front();
	}

	T* back()
	{
		return m_array.back();
	}

	const T* front() const
	{
		return m_array.front();
	}

	const T* back() const
	{
		return m_array.back();
	}

	void push_back(T* ptr)
	{
		m_array.push_back(ptr);
		if (ptr)
			ptr->AddRef();
	}

	void pop_back()
	{
		T* ptr = m_array.back();
		m_array.pop_back();

		if (ptr)
			ptr->Release();
	}
	
	bool empty() const
	{
		return m_array.empty();
	}

	size_type size() const
	{
		return m_array.size();
	}

	void resize(size_type size)
	{
		if (size < m_array.size())
		{
			for (size_t i = size; i < m_array.size(); ++i)
			{
				if (m_array[i])
					m_array[i]->Release();
			}
		}
		m_array.resize(size);
	}

	size_type capacity() const
	{
		return m_array.capacity();
	}

	void reserve(size_type size)
	{
		m_array.reserve(size);
	}

	T* at(size_type index) const
	{
		return m_array[index];
	}

	element operator [] (size_type index)
	{
		return element(m_array, index);
	}

	ComRefArray< T >& operator = (const ComRefArray< T >& src)
	{
		for (typename container_type::const_iterator i = src.m_array.begin(); i != src.m_array.end(); ++i)
		{
			if (*i)
				(*i)->AddRef();
		}
		for (typename container_type::iterator i = m_array.begin(); i != m_array.end(); ++i)
		{
			if (*i)
				(*i)->Release();
		}
		m_array = src.m_array;
		return *this;
	}

private:
	container_type m_array;
};

}

#endif	// traktor_ComRef_H
