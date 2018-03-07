/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_TypeInfo_H
#define traktor_TypeInfo_H

#include <set>
#include <string>
#include "Core/Config.h"
#include "Core/Ref.h"

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

/*! \brief Force linker to keep reference to type.
 * \ingroup Core
 */
//@{

void T_DLLCLASS __forceLinkReference(const class TypeInfo& type);
#define T_FORCE_LINK_REF(CLASS) \
	traktor::__forceLinkReference(traktor::type_of< CLASS >());

//@}

/*! \brief Instance factory.
 * \ingroup Core
 */
class IInstanceFactory : public IRefCount
{
public:
	virtual ITypedObject* createInstance(void* memory) const = 0;
};

/*! \brief Default instance factory implementation.
 * \ingroup Core
 */
template < typename T >
class InstanceFactory : public RefCountImpl< IInstanceFactory >
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

/*! \brief Type information.
 * \ingroup Core
 */
class T_DLLCLASS TypeInfo
{
public:
	TypeInfo(
		const wchar_t* name,
		size_t size,
		int32_t version,
		bool editable,
		const TypeInfo* super,
		const IInstanceFactory* factory
	);

	virtual ~TypeInfo();

	/*! \brief Name of type.
	 *
	 * \return Type name.
	 */
	const wchar_t* getName() const { return m_name; }

	/*! \brief Size of type in bytes.
	 *
	 * \return Type size in bytes.
	 */
	size_t getSize() const { return m_size; }

	/*! \brief Version of type.
	 *
	 * \return Type version.
	 */
	int32_t getVersion() const { return m_version; }

	/*! \brief Editable type.
	 *
	 * \return True if type is editable.
	 */
	bool isEditable() const { return m_editable; }

	/*! \brief Return super type.
	 *
	 * \return Super type.
	 */
	const TypeInfo* getSuper() const { return m_super; }

	/*! \brief Instantiable type.
	 *
	 * \return True if type is Instantiable.
	 */
	bool isInstantiable() const { return m_factory != 0; }

	/*! \brief Create new instance of type.
	 *
	 * \param memory Optional pointer to memory location.
	 * \return New instance.
	 */
	ITypedObject* createInstance(void* memory = 0) const;

	/*! \brief Find type from string representation.
	 *
	 * \return Type pointer, null if type not found.
	 */
	static const TypeInfo* find(const std::wstring& name);

	/*! \brief Find all types derived from this type.
	 *
	 * \param outTypes Found types.
	 * \param inclusive If this type should be included in result.
	 */
	void findAllOf(std::set< const TypeInfo* >& outTypes, bool inclusive = true) const;

	/*! \brief Create instance from type name.
	 *
	 * \param name Type name.
	 * \param memory Optional pointer to memory location.
	 * \return New instance.
	 */
	static ITypedObject* createInstance(const std::wstring& name, void* memory = 0);

	/*! \brief Set tag.
	 * \note This is specifically used for maintaining script class mapping.
	 */
	void setTag(uint32_t tag) const;

	/*! \brief Get tag.
	 */
	uint32_t getTag() const { return m_tag; }

private:
	const wchar_t* m_name;
	size_t m_size;
	int32_t m_version;
	bool m_editable;
	const TypeInfo* m_super;
	Ref< const IInstanceFactory > m_factory;
	mutable uint32_t m_tag;
};

/*! \brief Set of type information.
 * \ingroup Core
 */
typedef std::set< const TypeInfo* > TypeInfoSet;

/*! \brief Create type info set from single type.
 * \ingroup Core
 */
inline TypeInfoSet makeTypeInfoSet(const TypeInfo& t1)
{
	TypeInfoSet typeSet;
	typeSet.insert(&t1);
	return typeSet;
}

/*! \brief Create type info set from single type.
 * \ingroup Core
 */
inline TypeInfoSet makeTypeInfoSet(const TypeInfo& t1, const TypeInfo& t2)
{
	TypeInfoSet typeSet;
	typeSet.insert(&t1);
	typeSet.insert(&t2);
	return typeSet;
}

/*! \brief Create type info set from single type.
 * \ingroup Core
 */
inline TypeInfoSet makeTypeInfoSet(const TypeInfo& t1, const TypeInfo& t2, const TypeInfo& t3)
{
	TypeInfoSet typeSet;
	typeSet.insert(&t1);
	typeSet.insert(&t2);
	typeSet.insert(&t3);
	return typeSet;
}

/*! \brief Check if type is identical.
 * \ingroup Core
 */
inline bool is_type_a(const TypeInfo& base, const TypeInfo& type)
{
	return &base == &type;
}

/*! \brief Check if type is derived from a base type.
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

/*! \brief Return type difference.
 * \ingroup Core
 */
inline uint32_t type_difference(const TypeInfo& base, const TypeInfo& type)
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

//@}

}

#endif	// traktor_TypeInfo_H
