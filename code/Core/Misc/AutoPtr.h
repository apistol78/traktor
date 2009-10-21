#ifndef traktor_AutoPtr_H
#define traktor_AutoPtr_H

namespace traktor
{

	template < typename Type >
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
				delete m_ptr;
				m_ptr = 0;
			}
		}

		Type* ptr()
		{
			return m_ptr;
		}

		void reset(Type* ptr)
		{
			release();
			m_ptr = ptr;
		}

		Type* operator -> ()
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
