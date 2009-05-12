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

		AutoPtr(Type* ptr_)
		:	m_ptr(ptr_)
		{
		}

		AutoPtr(AutoPtr< Type >& lh)
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

		AutoPtr< Type >& operator = (Type* ptr_)
		{
			release();
			m_ptr = ptr_;
			return *this;
		}

		AutoPtr< Type >& operator = (AutoPtr< Type >& lh)
		{
			release();
			m_ptr = lh.m_ptr; lh.m_ptr = 0;
			return *this;
		}

		Type* operator -> ()
		{
			return ptr();
		}

	private:
		Type* m_ptr;
	};

	template < typename Type >
	class AutoArrayPtr
	{
	public:
		AutoArrayPtr()
		:	m_ptr(0)
		{
		}

		AutoArrayPtr(Type* ptr_)
		:	m_ptr(ptr_)
		{
		}

		AutoArrayPtr(AutoArrayPtr< Type >& lh)
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

		AutoArrayPtr< Type >& operator = (Type* ptr_)
		{
			release();
			m_ptr = ptr_;
			return *this;
		}

		AutoArrayPtr< Type >& operator = (AutoArrayPtr< Type >& lh)
		{
			release();
			m_ptr = lh.m_ptr; lh.m_ptr = 0;
			return *this;
		}

		Type& operator [] (int index)
		{
			return m_ptr[index];
		}

	private:
		Type* m_ptr;
	};

}

#endif	// traktor_AutoPtr_H
