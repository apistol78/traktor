#include <cstring>
#include "Script/Any.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/BlockAllocator.h"
#include "Core/Misc/String.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Singleton/SingletonManager.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

wchar_t* refStringCreate(const wchar_t* s)
{
	uint32_t len = wcslen(s);
	
	void* ptr = Alloc::acquire(sizeof(uint16_t) + (len + 1) * sizeof(wchar_t));
	if (!ptr)
		return 0;

	uint16_t* base = static_cast< uint16_t* >(ptr);
	*base = 0;

	wchar_t* c = reinterpret_cast< wchar_t* >(base + 1);
	if (len > 0)
		std::memcpy(c, s, len * sizeof(wchar_t));

	c[len] = L'\0';
	return c;
}

wchar_t* refStringInc(wchar_t* s)
{
	uint16_t* base = reinterpret_cast< uint16_t* >(s) - 1; *base++;
	return s;
}

wchar_t* refStringDec(wchar_t* s)
{
	uint16_t* base = reinterpret_cast< uint16_t* >(s) - 1;
	if (--*base == 0)
	{
		Alloc::free(base);
		return 0;
	}
	return s;
}

class RefPool : public ISingleton
{
public:
	enum { MaxRefCount = 1024 };

	static RefPool& getInstance()
	{
		static RefPool* s_instance = 0;
		if (!s_instance)
		{
			s_instance = new RefPool();
			SingletonManager::getInstance().add(s_instance);
		}
		return *s_instance;
	}

	Ref< Object >* construct(Object* obj)
	{
		void* ptr = m_alloc->alloc();
		T_FATAL_ASSERT_M (ptr, L"Out of memory");

		return new (ptr) Ref< Object > (obj);
	}

	void destruct(Ref< Object >* refPtr)
	{
		if (refPtr)
		{
			refPtr->~Ref< Object >();
			m_alloc->free(refPtr);
		}
	}

protected:
	RefPool()
	:	m_pool(0)
	,	m_alloc(0)
	{
		m_pool = Alloc::acquireAlign(MaxRefCount * sizeof(Ref< Object >), 16);
		m_alloc = new BlockAllocator(m_pool, MaxRefCount, sizeof(Ref< Object >));
	}

	virtual ~RefPool()
	{
		delete m_alloc;
		Alloc::freeAlign(m_pool);
	}

	virtual void destroy()
	{
		delete this;
	}

private:
	void* m_pool;
	BlockAllocator* m_alloc;
};

		}

Any::Any()
:	m_type(AtVoid)
{
}

Any::Any(const Any& src)
:	m_type(src.m_type)
{
	if (m_type == AtString)
		m_data.m_string = refStringInc(src.m_data.m_string);
	else if (m_type == AtObject)
		m_data.m_object = RefPool::getInstance().construct(*src.m_data.m_object);
	else
		m_data = src.m_data;
}

Any::Any(bool value)
:	m_type(AtBoolean)
{
	m_data.m_boolean = value;
}

Any::Any(int32_t value)
:	m_type(AtInteger)
{
	m_data.m_integer = value;
}

Any::Any(float value)
:	m_type(AtFloat)
{
	m_data.m_float = value;
}

Any::Any(const std::wstring& value)
:	m_type(AtString)
{
	m_data.m_string = refStringCreate(value.c_str());
}

Any::Any(Object* value)
:	m_type(AtObject)
{
	m_data.m_object = RefPool::getInstance().construct(value);
}

Any::~Any()
{
	T_EXCEPTION_GUARD_BEGIN
	
	if (m_type == AtString)
		refStringDec(m_data.m_string);
	else if (m_type == AtObject)
		RefPool::getInstance().destruct(m_data.m_object);

	T_EXCEPTION_GUARD_END
}

bool Any::getBoolean() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean;
	case AtInteger:
		return m_data.m_integer != 0;
	case AtFloat:
		return m_data.m_float != 0.0f;
	case AtString:
		return parseString< int32_t >(m_data.m_string) != 0;
	case AtObject:
		return *m_data.m_object != 0;
	}
	return false;
}

int32_t Any::getInteger() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean ? 1 : 0;
	case AtInteger:
		return m_data.m_integer;
	case AtFloat:
		return int32_t(m_data.m_float);
	case AtString:
		return parseString< int32_t >(m_data.m_string);
	}
	return 0;
}

float Any::getFloat() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean ? 1.0f : 0.0f;
	case AtInteger:
		return float(m_data.m_integer);
	case AtFloat:
		return m_data.m_float;
	case AtString:
		return parseString< float >(m_data.m_string);
	}
	return 0.0f;
}

std::wstring Any::getString() const
{
	switch (m_type)
	{
	case AtBoolean:
		return m_data.m_boolean ? L"true" : L"false";
	case AtInteger:
		return toString(m_data.m_integer);
	case AtFloat:
		return toString(m_data.m_float);
	case AtString:
		return m_data.m_string;
	}
	return L"";
}

Ref< Object > Any::getObject() const
{
	return m_type == AtObject ? *m_data.m_object : 0;
}

Any& Any::operator = (const Any& src)
{
	if (m_type == AtString)
		refStringDec(m_data.m_string);
	else if (m_type == AtObject)
		RefPool::getInstance().destruct(m_data.m_object);

	m_type = src.m_type;
	m_data = src.m_data;

	if (m_type == AtString)
		refStringInc(m_data.m_string);
	else if (m_type == AtObject)
		m_data.m_object = RefPool::getInstance().construct(*m_data.m_object);

	return *this;
}

	}
}
