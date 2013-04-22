#ifndef traktor_TypeInfo_H
#define traktor_TypeInfo_H

#include <set>
#include <vector>
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
	virtual ITypedObject* createInstance() const = 0;
};

/*! \brief Default instance factory implementation.
 * \ingroup Core
 */
template < typename T >
class InstanceFactory : public RefCountImpl< IInstanceFactory >
{
public:
	virtual ITypedObject* createInstance() const
	{
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
	const wchar_t* getName() const;

	/*! \brief Size of type in bytes.
	 *
	 * \return Type size in bytes.
	 */
	size_t getSize() const;

	/*! \brief Version of type.
	 *
	 * \return Type version.
	 */
	int32_t getVersion() const;

	/*! \brief Editable type.
	 *
	 * \return True if type is editable.
	 */
	bool isEditable() const;

	/*! \brief Return super type.
	 *
	 * \return Super type.
	 */
	const TypeInfo* getSuper() const;

	/*! \brief Instantiable type.
	 *
	 * \return True if type is Instantiable.
	 */
	bool isInstantiable() const;

	/*! \brief Create new instance of type.
	 *
	 * \return New instance.
	 */
	ITypedObject* createInstance() const;

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

private:
	const wchar_t* m_name;
	size_t m_size;
	int32_t m_version;
	bool m_editable;
	const TypeInfo* m_super;
	Ref< const IInstanceFactory > m_factory;
};

/*! \brief Set of type information.
 * \ingroup Core
 */
typedef std::set< const TypeInfo* > TypeInfoSet;

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
