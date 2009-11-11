#include <string>
#include <stdlib.h>
#include "Core/Rtti/Type.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{

static uint32_t s_typeCount = 0;
static uint32_t s_typeRegistrySize = 0;
static const Type** s_typeRegistry = 0;

	}

void __registerType(const Type* type)
{
	if (!s_typeRegistry)
	{
		s_typeRegistrySize = 1024;
		s_typeRegistry = reinterpret_cast< const Type** >(malloc(s_typeRegistrySize * sizeof(const Type*)));
		T_ASSERT (s_typeRegistry);
	}

	std::wstring typeName(type->getName());

	uint32_t index = 0;
	while (index < s_typeCount)
	{
		const Type* ttype = s_typeRegistry[index];
		T_ASSERT (type);

		std::wstring ttypeName(ttype->getName());
		int res = ttypeName.compare(typeName);
		if (res > 0)
			break;

		T_ASSERT_M (res, L"Type already defined");
		index++;
	}

	if (index < s_typeCount)
	{
		for (uint32_t i = s_typeCount; i > index; --i)
			s_typeRegistry[i] = s_typeRegistry[i - 1];
	}

	s_typeRegistry[index] = type;

	if (++s_typeCount >= s_typeRegistrySize)
	{
		s_typeRegistrySize += 1024;
		s_typeRegistry = reinterpret_cast< const Type** >(realloc(s_typeRegistry, s_typeRegistrySize * sizeof(const Type*)));
		T_ASSERT (s_typeRegistry);
	}
}

void __unregisterType(const Type* type)
{
}

Type::Type(const Type* super, const wchar_t* name, size_t size, const ObjectFactory* factory)
:	m_super(super)
,	m_name(name)
,	m_size(size)
,	m_factory(factory)
{
	__registerType(this);
}

Type::~Type()
{
	T_EXCEPTION_GUARD_BEGIN

	__unregisterType(this);
	delete m_factory;

	T_EXCEPTION_GUARD_END
}

const Type* Type::getSuper() const
{
	return m_super;
}

const wchar_t* Type::getName() const
{
	return m_name;
}

size_t Type::getSize() const
{
	return m_size;
}

bool Type::isInstantiable() const
{
	return m_factory != 0;
}

bool Type::isEditable() const
{
	return m_factory && m_factory->isEditable();
}

Ref< Object > Type::newInstance() const
{
	if (m_factory)
		return m_factory->newInstance();
	else
		return 0;
}

const Type* Type::find(const std::wstring& name)
{
	uint32_t first = 0;
	uint32_t last = s_typeCount;

	while (first < last)
	{
		uint32_t index = (last + first) >> 1;

		int res = std::wstring(s_typeRegistry[index]->getName()).compare(name);
		if (res == 0)
			return s_typeRegistry[index];
		else if (res > 0)
			last = index;
		else if (res < 0)
			first = index + 1;
	}

	return 0;
}

void Type::findAllOf(std::vector< const Type* >& outTypes, bool inclusive) const
{
	for (size_t i = 0; i < s_typeCount; ++i)
	{
		if (is_type_of(*this, *s_typeRegistry[i]))
		{
			if (inclusive || s_typeRegistry[i] != this)
				outTypes.push_back(s_typeRegistry[i]);
		}
	}
}

void __forceLinkReference(const Type& type)
{
	wchar_t* dummy = static_cast< wchar_t* >(alloca(256 * sizeof(wchar_t)));
#if defined(_WIN32)
	wcscpy_s(dummy, 256, type.getName());
#else
	wcscpy(dummy, type.getName());
#endif
}

}
