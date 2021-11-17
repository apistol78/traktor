#pragma once

#include "Core/Config.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Meta/Traits.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ITypedObject;
class TypeInfo;

/*! Instance factory.
 * \ingroup Core
 */
class IInstanceFactory
{
public:
	virtual ~IInstanceFactory() {}

	virtual ITypedObject* createInstance(void* memory) const = 0;
};

/*! Specialized instance factory implementation.
 * \ingroup Core
 */
template < typename T >
class InstanceFactory : public IInstanceFactory
{
public:
	virtual ITypedObject* createInstance(void* memory) const
	{
		if (memory)
			return new (memory) T();
		else
			return new T();
	}
};

/*! Set of type information.
 * \ingroup Core
 */
class T_DLLCLASS TypeInfoSet : public SmallSet< const TypeInfo* >
{
public:
	template < typename Type >
	bool insert()
	{
		typedef typename IsPointer< typename IsReference< Type >::base_t >::base_t tt;
		return insert(&(tt::getClassTypeInfo()));
	}

	bool insert(const TypeInfo* typeInfo) { return SmallSet< const TypeInfo* >::insert(typeInfo); }

	uint32_t insert(const const_iterator& from, const const_iterator& to) { return SmallSet< const TypeInfo* >::insert(from, to); }
};

/*! Type information.
 * \ingroup Core
 */
class T_DLLCLASS TypeInfo
{
public:
	explicit TypeInfo(
		const wchar_t* name,
		uint32_t size,
		int32_t version,
		bool editable,
		const TypeInfo* super,
		const IInstanceFactory* factory
	);

	virtual ~TypeInfo();

	/*! Name of type.
	 *
	 * \return Type name.
	 */
	const wchar_t* getName() const { return m_name; }

	/*! Size of type in bytes.
	 *
	 * \return Type size in bytes.
	 */
	uint32_t getSize() const { return m_size; }

	/*! Version of type.
	 *
	 * \return Type version.
	 */
	int32_t getVersion() const { return m_version; }

	/*! Editable type.
	 *
	 * \return True if type is editable.
	 */
	bool isEditable() const { return m_editable; }

	/*! Return super type.
	 *
	 * \return Super type.
	 */
	const TypeInfo* getSuper() const { return m_super; }

	/*! Instantiable type.
	 *
	 * \return True if type is Instantiable.
	 */
	bool isInstantiable() const { return m_factory != 0; }

	/*! Create new instance of type.
	 *
	 * \param memory Optional pointer to memory location.
	 * \return New instance.
	 */
	ITypedObject* createInstance(void* memory = 0) const;

	/*! Find type from string representation.
	 *
	 * \return Type pointer, null if type not found.
	 */
	static const TypeInfo* find(const wchar_t* name);

	/*! Find all types derived from this type.
	 *
	 * \param inclusive If this type should be included in result.
	 * \return Found types.
	 */
	TypeInfoSet findAllOf(bool inclusive = true) const;

	/*! Create instance from type name.
	 *
	 * \param name Type name.
	 * \param memory Optional pointer to memory location.
	 * \return New instance.
	 */
	static ITypedObject* createInstance(const wchar_t* name, void* memory = 0);

	/*! Set tag.
	 * \note This is specifically used for maintaining script class mapping.
	 */
	void setTag(uint32_t tag) const;

	/*! Get tag.
	 */
	uint32_t getTag() const { return m_tag; }

private:
	const wchar_t* m_name;
	uint32_t m_size;
	int32_t m_version;
	bool m_editable;
	const TypeInfo* m_super;
	const IInstanceFactory* m_factory;
	mutable uint32_t m_tag;
};

/*! Create type info set from single type.
 * \ingroup Core
 */
TypeInfoSet T_DLLCLASS makeTypeInfoSet(const TypeInfo& t1);

/*! Create type info set from single type.
 * \ingroup Core
 */
TypeInfoSet T_DLLCLASS makeTypeInfoSet(const TypeInfo& t1, const TypeInfo& t2);

/*! Create type info set from single type.
 * \ingroup Core
 */
TypeInfoSet T_DLLCLASS makeTypeInfoSet(const TypeInfo& t1, const TypeInfo& t2, const TypeInfo& t3);

/*! Check if type is identical.
 * \ingroup Core
 */
inline bool is_type_a(const TypeInfo& base, const TypeInfo& type)
{
	return &base == &type;
}

/*! Check if type is derived from a base type.
 * \ingroup Core
 */
inline bool is_type_of(const TypeInfo& base, const TypeInfo& type)
{
	if (is_type_a(base, type))
		return true;

	if (!type.getSuper())
		return false;

	return is_type_of(base, *type.getSuper());
}

/*! Return type difference.
 * \ingroup Core
 */
uint32_t T_DLLCLASS type_difference(const TypeInfo& base, const TypeInfo& type);


/*! Force linker to keep reference to type.
 * \ingroup Core
 */
//@{

void T_DLLCLASS __forceLinkReference(const class TypeInfo& type);
#define T_FORCE_LINK_REF(CLASS) \
	traktor::__forceLinkReference(traktor::type_of< CLASS >());

//@}

}
