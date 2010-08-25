#ifndef traktor_AutoPtr_H
#define traktor_AutoPtr_H

#include "Core/Config.h"
#include "Core/Memory/Alloc.h"

namespace traktor
{

	/*! \brief Default delete operator policy.
	 * \ingroup Core
	 */
	template < typename Type >
	struct DeleteOperator
	{
		static void release(Type* ptr)
		{
			delete ptr;
		}
	};

	/*! \brief Use freeAlign method to release memory.
	 * \ingroup Core
	 */
	template < typename Type >
	struct AllocFreeAlign
	{
		static void release(Type* ptr)
		{
			Alloc::freeAlign(ptr);
		}
	};

	/*! \brief Auto pointer
	 * \ingroup Core
	 */
	template < typename Type, typename ReleasePolicy = DeleteOperator< Type > >
	class AutoPtr
	{
	public:
		AutoPtr()
		:	m_ptr(0)
		{
		}

		explicit AutoPtr(Type* ptr_)
		:	m_ptr(ptr_)
		{
		}

		explicit AutoPtr(AutoPtr< Type >& lh)
		{
			m_ptr = lh.m_ptr;
			lh.m_ptr = 0;
		}

		virtual ~AutoPtr()
		{
			release();
		}

		void release()
		{
			if (m_ptr)
			{
				ReleasePolicy::release(m_ptr);
				m_ptr = 0;
			}
		}

		Type* ptr() const
		{
			return m_ptr;
		}

		void reset(Type* ptr)
		{
			release();
			m_ptr = ptr;
		}

		Type* operator -> () const
		{
			return ptr();
		}

	private:
		Type* m_ptr;
		
		AutoPtr< Type >& operator = (const AutoPtr< Type >& lh) { T_FATAL_ERROR; return *this; }
	};

	template < typename Type >
	class AutoArrayPtr
	{
	public:
		AutoArrayPtr()
		:	m_ptr(0)
		{
		}

		explicit AutoArrayPtr(Type* ptr_)
		:	m_ptr(ptr_)
		{
		}

		explicit AutoArrayPtr(AutoArrayPtr< Type >& lh)
		{
			m_ptr = lh.m_ptr;
			lh.m_ptr = 0;
		}

		virtual ~AutoArrayPtr()
		{
			release();
		}

		void release()
		{
			if (m_ptr)
			{
				delete[] m_ptr;
				m_ptr = 0;
			}
		}

		Type* ptr()
		{
			return m_ptr;
		}

		const Type* c_ptr() const
		{
			return m_ptr;
		}
		
		void reset(Type* ptr)
		{
			release();
			m_ptr = ptr;
		}

		Type& operator [] (int index)
		{
			return m_ptr[index];
		}

		const Type& operator [] (int index) const
		{
			return m_ptr[index];
		}

	private:
		Type* m_ptr;
		
		AutoArrayPtr& operator = (const AutoArrayPtr< Type >& lh) { T_FATAL_ERROR; return *this; }
	};

}

#endif	// traktor_AutoPtr_H
