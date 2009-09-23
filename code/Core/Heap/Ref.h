#ifndef traktor_Ref_H
#define traktor_Ref_H

#include <vector>
#include <list>
#include "Core/Config.h"
#include "Core/Heap/Heap.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Acquire.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class Heap;
class Object;

/*! \brief Reference interface.
 * \ingroup Core
 *
 * All reference containers must implement this interface
 * in order for the heap to be able to traverse live objects.
 */
class T_DLLCLASS RefBase
{
public:
	Object* m_owner;
	RefBase* m_prev;
	RefBase* m_next;
#if defined(T_DEBUG_REFERENCE_ADDRESS)
	uint32_t m_adress;
#endif

	struct Visitor
	{
		virtual ~Visitor() {}

		virtual void operator () (Object* object) = 0;
	};

	RefBase()
	:	m_owner(0)
	,	m_prev(0)
	,	m_next(0)
#if defined(T_DEBUG_REFERENCE_ADDRESS)
	,	m_adress(0UL)
#endif
	{
	}
	
	virtual ~RefBase()
	{
	}

	virtual void visit(Visitor& visitor) = 0;

	virtual void invalidate() = 0;

	virtual void invalidate(Object* ptr) = 0;
};

/*! \brief Single object reference.
 * \ingroup Core
 */
class T_DLLCLASS RefSingleBase : public RefBase
{
public:
#if defined(T_DEBUG_REFERENCES)
	enum PointerType
	{
		PtNull,
		PtManaged,
		PtUnmanaged
	};
#endif

	RefSingleBase(Object* ptr)
	:	RefBase()
	,	m_ptr(ptr)
	{
#if defined(T_DEBUG_REFERENCES)
		updatePointerType();
#endif
	}

	void setPtr(Object* ptr)
	{
		m_ptr = ptr;
#if defined(T_DEBUG_REFERENCES)
		updatePointerType();
#endif
	}

	Object* getPtr()
	{
		return m_ptr;
	}

	Object* getPtr() const
	{
		return m_ptr;
	}

protected:
	virtual void visit(Visitor& visitor)
	{
		if (m_ptr)
			visitor(m_ptr);
	}

	virtual void invalidate()
	{
		m_ptr = 0;
#if defined(T_DEBUG_REFERENCES)
		updatePointerType();
#endif
	}

	virtual void invalidate(Object* ptr)
	{
		if (m_ptr == ptr)
		{
			m_ptr = 0;
#if defined(T_DEBUG_REFERENCES)
			updatePointerType();
#endif
		}
	}

#if defined(T_DEBUG_REFERENCES)
	T_FORCE_INLINE void updatePointerType()
	{
		if (m_ptr)
			m_ptrType = m_ptr->isManaged() ? PtManaged : PtUnmanaged;
		else
			m_ptrType = PtNull;
	}
#endif

private:
	Object* m_ptr;
#if defined(T_DEBUG_REFERENCES)
	PointerType m_ptrType;
#endif
};

/*! \brief Single object reference container.
 * \ingroup Core
 */
template < typename T > class Ref : public RefSingleBase
{
public:
	Ref< T >()
	:	RefSingleBase(0)
	{
		Heap::getInstance().addRef(this);
	}

	Ref< T >(T* ptr)
	:	RefSingleBase((Object*)(ptr))
	{
		Heap::getInstance().addRef(this);
	}

	Ref< T >(const Ref< T >& ref)
	:	RefSingleBase((Object*)(ref.getPtr()))
	{
		Heap::getInstance().addRef(this);
	}

	virtual ~Ref< T >()
	{
		Heap::getInstance().removeRef(this);
	}
	
	Ref< T >& operator = (T* ptr)
	{
		setPtr((Object*)(ptr));
		return *this;
	}

	Ref< T >& operator = (const Ref< T >& ref)
	{
		setPtr((Object*)(ref.getPtr()));
		return *this;
	}
	
	T* getPtr()
	{
		return reinterpret_cast< T* >(RefSingleBase::getPtr());
	}

	T* getPtr() const
	{
		return reinterpret_cast< T* >(RefSingleBase::getPtr());
	}

	operator T* () const
	{
		return getPtr();
	}

	T& operator * () const
	{
		return *getPtr();
	}

	T* operator -> () const
	{
		return getPtr();
	}
};

/*! \brief Specialized reference container for base type.
 * \ingroup Core
 */
template <> class Ref < Object > : public RefSingleBase
{
public:
	Ref< Object >()
	:	RefSingleBase(0)
	{
		Heap::getInstance().addRef(this);
	}

	Ref< Object >(Object* ptr)
	:	RefSingleBase(ptr)
	{
		Heap::getInstance().addRef(this);
	}

	Ref< Object >(const Ref< Object >& ref)
	:	RefSingleBase(ref.getPtr())
	{
		Heap::getInstance().addRef(this);
	}

	virtual ~Ref< Object >()
	{
		Heap::getInstance().removeRef(this);
	}
	
	Ref< Object >& operator = (Object* ptr)
	{
		setPtr(ptr);
		return *this;
	}

	Ref< Object >& operator = (const Ref< Object >& ref)
	{
		setPtr(ref.getPtr());
		return *this;
	}

	operator Object* () const
	{
		return getPtr();
	}

	Object& operator * () const
	{
		return *getPtr();
	}

	Object* operator -> () const
	{
		return getPtr();
	}
};

/*! \brief Object array reference container.
 * \ingroup Core
 */
template < typename Class >
class RefArray : public RefBase
{
public:
	typedef std::vector< Class* > container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;
	typedef Semaphore lock_type;
	typedef Acquire< lock_type > lock_rd_type;
	typedef Acquire< lock_type > lock_wr_type;

	RefArray()
	:	RefBase()
	{
		Heap::getInstance().addRef(this);
	}

	RefArray(size_type size)
	:	RefBase()
	,	m_array(size)
	{
		Heap::getInstance().addRef(this);
	}

	RefArray(const RefArray< Class >& s)
	:	RefBase()
	,	m_array(s.m_array)
	{
		Heap::getInstance().addRef(this);
	}

	virtual ~RefArray()
	{
		Heap::getInstance().removeRef(this);
	}

	void clear()
	{
		lock_wr_type sc(m_lock);
		m_array.clear();
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

	Class*& front()
	{
		return m_array.front();
	}

	Class*& back()
	{
		return m_array.back();
	}

	const Class* front() const
	{
		return m_array.front();
	}

	const Class* back() const
	{
		return m_array.back();
	}

	void push_back(Class* const val)
	{
		lock_wr_type sc(m_lock);
		m_array.push_back(val);
	}

	void push_back_unsafe(Class* const val)
	{
		m_array.push_back(val);
	}

	void pop_back()
	{
		lock_wr_type sc(m_lock);
		m_array.pop_back();
	}

	void pop_back_unsafe()
	{
		m_array.pop_back();
	}

	void insert(iterator at, Class* const val)
	{
		lock_wr_type sc(m_lock);
		m_array.insert(at, val);
	}

	void insert(iterator at, iterator first, iterator last)
	{
		lock_wr_type sc(m_lock);
		m_array.insert(at, first, last);
	}

	iterator erase(iterator iter)
	{
		lock_wr_type sc(m_lock);
		return m_array.erase(iter);
	}

	iterator erase(iterator first, iterator last)
	{
		lock_wr_type sc(m_lock);
		return m_array.erase(first, last);
	}

	void swap(RefArray< Class >& src)
	{
		lock_wr_type sc1(m_lock);
		lock_wr_type sc2(src.m_lock);
		m_array.swap(src.m_array);
	}

	bool empty() const
	{
		lock_rd_type sc(m_lock);
		return m_array.empty();
	}

	size_type size() const
	{
		lock_rd_type sc(m_lock);
		return m_array.size();
	}

	void resize(size_type size)
	{
		lock_wr_type sc(m_lock);
		m_array.resize(size);
	}

	size_type capacity() const
	{
		lock_rd_type sc(m_lock);
		return m_array.capacity();
	}

	void reserve(size_type size)
	{
		lock_wr_type sc(m_lock);
		m_array.reserve(size);
	}

	Class*& at(size_type index)
	{
		return m_array[index];
	}

	Class* at(size_type index) const
	{
		return m_array[index];
	}

	Class*& operator [] (size_type index)
	{
		return m_array[index];
	}

	Class* operator [] (size_type index) const
	{
		return m_array[index];
	}

	RefArray< Class >& operator = (const RefArray< Class >& src)
	{
		lock_wr_type sc1(m_lock);
		lock_rd_type sc2(src.m_lock);
		m_array = src.m_array;
		return *this;
	}

	lock_type& lock()
	{
		return m_lock;
	}

protected:
	virtual void visit(Visitor& visitor)
	{
		lock_rd_type sc(m_lock);
		for (iterator i = begin(); i != end(); ++i)
		{
			if (*i)
				visitor((Object*)(*i));
		}
	}

	virtual void invalidate()
	{
		lock_wr_type sc(m_lock);
		for (iterator i = begin(); i != end(); ++i)
			*i = 0;
	}

	virtual void invalidate(Object* ptr)
	{
		lock_wr_type sc(m_lock);
		for (iterator i = begin(); i != end(); ++i)
		{
			if ((Object*)(*i) == ptr)
				*i = 0;
		}
	}

private:
	container_type m_array;
	mutable lock_type m_lock;
};

/*! \brief Object list reference container.
 * \ingroup Core
 */
template < typename Class >
class RefList : public RefBase
{
public:
	typedef std::list< Class* > container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;
	typedef Semaphore lock_type;
	typedef Acquire< lock_type > lock_rd_type;
	typedef Acquire< lock_type > lock_wr_type;

	RefList()
	:	RefBase()
	{
		Heap::getInstance().addRef(this);
	}

	RefList(size_type size)
	:	RefBase()
	,	m_list(size)
	{
		Heap::getInstance().addRef(this);
	}

	RefList(const RefList< Class >& s)
	:	RefBase()
	,	m_list(s.m_list)
	{
		Heap::getInstance().addRef(this);
	}

	virtual ~RefList()
	{
		Heap::getInstance().removeRef(this);
	}

	void clear()
	{
		lock_wr_type sc(m_lock);
		m_list.clear();
	}

	iterator begin()
	{
		return m_list.begin();
	}

	iterator end()
	{
		return m_list.end();
	}

	const_iterator begin() const
	{
		return m_list.begin();
	}

	const_iterator end() const
	{
		return m_list.end();
	}

	Class*& front()
	{
		return m_list.front();
	}

	Class*& back()
	{
		return m_list.back();
	}

	const Class* front() const
	{
		return m_list.front();
	}

	const Class* back() const
	{
		return m_list.back();
	}

	void push_back(Class* const val)
	{
		lock_wr_type sc(m_lock);
		m_list.push_back(val);
	}

	void pop_back()
	{
		lock_wr_type sc(m_lock);
		m_list.pop_back();
	}

	void push_front(Class* const val)
	{
		lock_wr_type sc(m_lock);
		m_list.push_front(val);
	}

	void pop_front()
	{
		lock_wr_type sc(m_lock);
		m_list.pop_front();
	}

	void insert(iterator at, Class* const val)
	{
		lock_wr_type sc(m_lock);
		m_list.insert(at, val);
	}

	void insert(iterator at, iterator first, iterator last)
	{
		lock_wr_type sc(m_lock);
		m_list.insert(at, first, last);
	}

	iterator erase(iterator iter)
	{
		lock_wr_type sc(m_lock);
		return m_list.erase(iter);
	}

	iterator erase(iterator first, iterator last)
	{
		lock_wr_type sc(m_lock);
		return m_list.erase(first, last);
	}

	void remove(Class* const val)
	{
		lock_wr_type sc(m_lock);
		m_list.remove(val);
	}

	void swap(RefList< Class >& src)
	{
		lock_wr_type sc1(m_lock);
		lock_wr_type sc2(src.m_lock);
		m_list.swap(src.m_list);
	}

	bool empty() const
	{
		lock_rd_type sc(m_lock);
		return m_list.empty();
	}

	size_type size() const
	{
		lock_rd_type sc(m_lock);
		return m_list.size();
	}

	void resize(size_type size)
	{
		lock_wr_type sc(m_lock);
		m_list.resize(size);
	}

	size_type capacity() const
	{
		lock_rd_type sc(m_lock);
		return m_list.capacity();
	}

	void reserve(size_type size)
	{
		lock_wr_type sc(m_lock);
		m_list.reserve(size);
	}

	void sort()
	{
		lock_wr_type sc(m_lock);
		m_list.sort();
	}

	template < class Traits >
	void sort(Traits comp)
	{
		lock_wr_type sc(m_lock);
		m_list.sort< Traits >(comp);
	}

	RefList< Class >& operator = (const RefList< Class >& src)
	{
		lock_wr_type sc1(m_lock);
		lock_rd_type sc2(src.m_lock);
		m_list = src.m_list;
		return *this;
	}

	lock_type& lock()
	{
		return m_lock;
	}

protected:
	virtual void visit(Visitor& visitor)
	{
		lock_rd_type sc(m_lock);
		for (iterator i = begin(); i != end(); ++i)
		{
			if (*i)
				visitor((Object*)(*i));
		}
	}

	virtual void invalidate()
	{
		lock_wr_type sc(m_lock);
		for (iterator i = begin(); i != end(); ++i)
			*i = 0;
	}

	virtual void invalidate(Object* ptr)
	{
		lock_wr_type sc(m_lock);
		for (iterator i = begin(); i != end(); ++i)
		{
			if ((Object*)(*i) == ptr)
				*i = 0;
		}
	}

private:
	container_type m_list;
	mutable lock_type m_lock;
};

/*! \brief Object set reference container.
 * \ingroup Core
 */
template < typename Class >
class RefSet : public RefBase
{
public:
	typedef std::set< Class* > container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::key_type key_type;
	typedef typename container_type::iterator iterator;
	typedef typename container_type::const_iterator const_iterator;
	typedef Semaphore lock_type;
	typedef Acquire< lock_type > lock_rd_type;
	typedef Acquire< lock_type > lock_wr_type;

	RefSet()
	:	RefBase()
	{
		Heap::getInstance().addRef(this);
	}

	RefSet(const RefSet< Class >& s)
	:	RefBase()
	,	m_set(s.m_set)
	{
		Heap::getInstance().addRef(this);
	}

	virtual ~RefSet()
	{
		Heap::getInstance().removeRef(this);
	}

	void clear()
	{
		lock_wr_type sc(m_lock);
		m_set.clear();
	}

	size_t count() const
	{
		lock_rd_type sc(m_lock);
		return m_set.count();
	}

	bool empty() const
	{
		lock_rd_type sc(m_lock);
		return m_set.empty();
	}

	iterator begin()
	{
		return m_set.begin();
	}

	iterator end()
	{
		return m_set.end();
	}

	const_iterator begin() const
	{
		return m_set.begin();
	}

	const_iterator end() const
	{
		return m_set.end();
	}

	void erase(iterator where)
	{
		lock_wr_type sc(m_lock);
		m_set.erase(where);
	}

	void erase(iterator first, iterator last)
	{
		lock_wr_type sc(m_lock);
		m_set.erase(first, last);
	}

	size_type erase(const key_type& val)
	{
		lock_wr_type sc(m_lock);
		return m_set.erase(val);
	}

	iterator find(const key_type& val)
	{
		lock_rd_type sc(m_lock);
		return m_set.find(val);
	}

	const_iterator find(const key_type& val) const
	{
		lock_rd_type sc(m_lock);
		return m_set.find(val);
	}

	void insert(Class* const val)
	{
		lock_wr_type sc(m_lock);
		m_set.insert(val);
	}

	template < typename InputIterator >
	void insert(InputIterator first, InputIterator last)
	{
		lock_wr_type sc(m_lock);
		for (InputIterator i = first; i != last; ++i)
			m_set.insert(*i);
	}

	size_type size() const
	{
		lock_rd_type sc(m_lock);
		return m_set.size();
	}

	void swap(RefSet< Class >& right)
	{
		lock_wr_type sc1(m_lock);
		lock_rd_type sc2(right.m_lock);
		m_set.swap(right.m_set);
	}

	lock_type& lock()
	{
		return m_lock;
	}

protected:
	virtual void visit(Visitor& visitor)
	{
		lock_rd_type sc(m_lock);
		for (iterator i = begin(); i != end(); ++i)
		{
			if (*i)
				visitor((Object*)(*i));
		}
	}

	virtual void invalidate()
	{
		lock_wr_type sc(m_lock);
		for (iterator i = begin(); i != end(); ++i)
			*i = 0;
	}

	virtual void invalidate(Object* ptr)
	{
		lock_wr_type sc(m_lock);
		for (iterator i = begin(); i != end(); ++i)
		{
			if ((Object*)(*i) == ptr)
				*i = 0;
		}
	}

private:
	container_type m_set;
	mutable lock_type m_lock;
};

}

#endif	// traktor_Ref_H
