/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_HvkRef_H
#define traktor_physics_HvkRef_H

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Havok
 */
template <typename T> struct HvkRef
{
public:
	HvkRef()
	:	m_ptr(0)
	{
	}

	explicit HvkRef(T* ptr)
	:	m_ptr(ptr)
	{
	}
	
	explicit HvkRef(const HvkRef<T>& src)
	:	m_ptr(src.m_ptr)
	{
		if (m_ptr != 0)
			m_ptr->addReference();
	}
	
	virtual ~HvkRef()
	{
		if (m_ptr != 0)
			m_ptr->removeReference();
	}

	inline HvkRef<T>& operator = (T* ptr)
	{
		if (m_ptr != 0)
			m_ptr->removeReference();
		m_ptr = ptr;
		return *this;
	}

	inline HvkRef<T>& operator = (const HvkRef<T>& src)
	{
		if (m_ptr != 0)
			m_ptr->removeReference();
		if ((m_ptr = src.m_ptr) != 0)
			m_ptr->addReference();
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
			m_ptr->removeReference();
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

	}
}

#endif	// traktor_physics_HvkRef_H
