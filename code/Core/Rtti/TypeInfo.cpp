/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
// If non zero then registry size is fixed.
#define T_REGISTRY_SIZE 16384

#if T_REGISTRY_SIZE != 0 && !defined(_WIN32)
#	include <alloca.h>
#endif
#include "Core/Misc/TString.h"
#include "Core/Rtti/TypeInfo.h"

namespace traktor
{
	namespace
	{

// Ensure RTTI registry is initialized early.
#if defined(_MSC_VER)
#	pragma warning(disable : 4073)
#	pragma init_seg(lib)
#endif

static uint32_t s_typeInfoCount = 0;
#if T_REGISTRY_SIZE == 0
static uint32_t s_typeInfoRegistrySize = 0;
static const TypeInfo** s_typeInfoRegistry = 0;
#else
static const TypeInfo* s_typeInfoRegistry[T_REGISTRY_SIZE];
#endif

int32_t safeStringCompare(const wchar_t* a, const wchar_t* b)
{
	int32_t ca = 0, cb = 0;
	for (;;)
	{
		ca = int32_t(*a++);
		cb = int32_t(*b++);
		if (ca != cb || ca == 0)
			break;
	}
	if (ca < cb)
		return -1;
	else if (ca > cb)
		return 1;
	else
		return 0;
}

	}

void __registerTypeInfo(const TypeInfo* typeInfo)
{
#if T_REGISTRY_SIZE == 0
	if (!s_typeInfoRegistry)
	{
		s_typeInfoRegistrySize = 1024;
		s_typeInfoRegistry = reinterpret_cast< const TypeInfo** >(std::malloc(s_typeInfoRegistrySize * sizeof(const TypeInfo*)));
		T_ASSERT(s_typeInfoRegistry);
	}
#endif

	const wchar_t* typeName = typeInfo->getName();
	if (!typeName)
		return;

	uint32_t index = 0;
	while (index < s_typeInfoCount)
	{
		const TypeInfo* typeInfo2 = s_typeInfoRegistry[index];
		T_ASSERT(typeInfo2);

		const wchar_t* typeName2 = typeInfo2->getName();
		const int32_t res = safeStringCompare(typeName2, typeName);
		if (res > 0)
			break;

		T_ASSERT_M (res, L"Type already defined");
		index++;
	}

	if (index < s_typeInfoCount)
	{
		for (uint32_t i = s_typeInfoCount; i > index; --i)
			s_typeInfoRegistry[i] = s_typeInfoRegistry[i - 1];
	}

	s_typeInfoRegistry[index] = typeInfo;

#if T_REGISTRY_SIZE == 0
	if (++s_typeInfoCount >= s_typeInfoRegistrySize)
	{
		s_typeInfoRegistrySize += 1024;
		s_typeInfoRegistry = reinterpret_cast< const TypeInfo** >(std::realloc(s_typeInfoRegistry, s_typeInfoRegistrySize * sizeof(const TypeInfo*)));
		T_ASSERT(s_typeInfoRegistry);
	}
#else
	++s_typeInfoCount;
#endif
}

void __unregisterTypeInfo(const TypeInfo* typeInfo)
{
	const wchar_t* typeName = typeInfo->getName();
	if (!typeName)
		return;

	uint32_t index = 0;
	while (index < s_typeInfoCount)
	{
		const TypeInfo* typeInfo2 = s_typeInfoRegistry[index];
		T_ASSERT(typeInfo2);

		const wchar_t* typeName2 = typeInfo2->getName();
		if (safeStringCompare(typeName2, typeName) == 0)
			break;

		index++;
	}
	T_ASSERT_M(index < s_typeInfoCount, L"Type not registered");

	for (uint32_t i = index; i < s_typeInfoCount - 1; ++i)
		s_typeInfoRegistry[i] = s_typeInfoRegistry[i + 1];

	--s_typeInfoCount;
}

TypeInfo::TypeInfo(
	const wchar_t* name,
	uint32_t size,
	int32_t version,
	bool editable,
	const TypeInfo* super,
	const IInstanceFactory* factory
)
:	m_name(name)
,	m_size(size)
,	m_version(version)
,	m_editable(editable)
,	m_super(super)
,	m_factory(factory)
,	m_tag(0)
{
	__registerTypeInfo(this);
}

TypeInfo::~TypeInfo()
{
	__unregisterTypeInfo(this);
	if (m_factory)
		delete m_factory;
}

ITypedObject* TypeInfo::createInstance(void* memory) const
{
	if (m_factory)
		return m_factory->createInstance(memory);
	else
		return nullptr;
}

const TypeInfo* TypeInfo::find(const wchar_t* name)
{
	uint32_t first = 0;
	uint32_t last = s_typeInfoCount;

	while (first < last)
	{
		const uint32_t index = (last + first) >> 1;
		T_ASSERT(index >= first && index < last);

		const wchar_t* typeName = s_typeInfoRegistry[index]->getName();
		const int32_t res = safeStringCompare(typeName, name);
		if (res == 0)
			return s_typeInfoRegistry[index];
		else if (res > 0)
			last = index;
		else if (res < 0)
			first = index + 1;
	}

	return nullptr;
}

TypeInfoSet TypeInfo::findAllOf(bool inclusive) const
{
	TypeInfoSet typeInfoSet;
	for (uint32_t i = 0; i < s_typeInfoCount; ++i)
	{
		if (is_type_of(*this, *s_typeInfoRegistry[i]))
		{
			if (inclusive || s_typeInfoRegistry[i] != this)
				typeInfoSet.insert(s_typeInfoRegistry[i]);
		}
	}
	return typeInfoSet;
}

ITypedObject* TypeInfo::createInstance(const wchar_t* name, void* memory)
{
	const TypeInfo* type = TypeInfo::find(name);
	return type ? type->createInstance(memory) : nullptr;
}

void TypeInfo::setTag(uint32_t tag) const
{
	m_tag = tag;
}

TypeInfoSet makeTypeInfoSet(const TypeInfo& t1)
{
	TypeInfoSet typeSet;
	typeSet.insert(&t1);
	return typeSet;
}

TypeInfoSet makeTypeInfoSet(const TypeInfo& t1, const TypeInfo& t2)
{
	TypeInfoSet typeSet;
	typeSet.insert(&t1);
	typeSet.insert(&t2);
	return typeSet;
}

TypeInfoSet makeTypeInfoSet(const TypeInfo& t1, const TypeInfo& t2, const TypeInfo& t3)
{
	TypeInfoSet typeSet;
	typeSet.insert(&t1);
	typeSet.insert(&t2);
	typeSet.insert(&t3);
	return typeSet;
}

TypeInfoSet makeTypeInfoSet(const TypeInfo& t1, const TypeInfo& t2, const TypeInfo& t3, const TypeInfo& t4)
{
	TypeInfoSet typeSet;
	typeSet.insert(&t1);
	typeSet.insert(&t2);
	typeSet.insert(&t3);
	typeSet.insert(&t4);
	return typeSet;
}

TypeInfoSet makeTypeInfoSet(const TypeInfo& t1, const TypeInfo& t2, const TypeInfo& t3, const TypeInfo& t4, const TypeInfo& t5)
{
	TypeInfoSet typeSet;
	typeSet.insert(&t1);
	typeSet.insert(&t2);
	typeSet.insert(&t3);
	typeSet.insert(&t4);
	typeSet.insert(&t5);
	return typeSet;
}

uint32_t type_difference(const TypeInfo& base, const TypeInfo& type)
{
	uint32_t difference = 0;

	// Traverse up in inheritance chain from until we reach base type.
	for (const TypeInfo* i = &type; i; i = i->getSuper(), ++difference)
	{
		if (i == &base)
			return difference;
	}

	// Unable to reach base type; add inheritance depth of base type.
	for (const TypeInfo* i = &base; i; i = i->getSuper())
		++difference;

	return difference;
}

void __forceLinkReference(const TypeInfo& type)
{
	wchar_t* dummy = static_cast< wchar_t* >(alloca(256 * sizeof(wchar_t)));
#	if defined(_WIN32)
	wcscpy_s(dummy, 256, type.getName());
#	else
	wcscpy(dummy, type.getName());
#	endif
}

}
