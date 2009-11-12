#ifndef traktor_kernel_Type_H
#define traktor_kernel_Type_H

#include <vector>
#include <set>
#include "Core/Config.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

/*! \ingroup Core */
//@{

extern void T_DLLCLASS __forceLinkReference(const class Type& type);
#define T_FORCE_LINK_REF(CLASS) \
	traktor::__forceLinkReference(traktor::type_of< CLASS >());

class Object;

#define T_RTTI_CLASS(CLASS) \
	public: \
		static const traktor::Type& getClassType(); \
		virtual const traktor::Type& getType() const; \
	private: \
		static const traktor::Type m__type__;

#define T_IMPLEMENT_RTTI_CLASS_NB(CLASSID, CLASS) \
	const traktor::Type CLASS::m__type__(0, CLASSID, sizeof(CLASS), 0); \
	\
	const traktor::Type& CLASS::getClassType() { \
		return m__type__; \
	} \
	const traktor::Type& CLASS::getType() const { \
		return m__type__; \
	}

#define T_IMPLEMENT_RTTI_CLASS(CLASSID, CLASS, SUPER) \
	const traktor::Type CLASS::m__type__(&traktor::type_of< SUPER >(), CLASSID, sizeof(CLASS), 0); \
	\
	const traktor::Type& CLASS::getClassType() { \
		return m__type__; \
	} \
	const traktor::Type& CLASS::getType() const { \
		return m__type__; \
	}

#define T_IMPLEMENT_RTTI_COMPOSITE_CLASS(CLASSID, OUTER, CLASS, SUPER) \
	const traktor::Type OUTER::CLASS::m__type__(&traktor::type_of< SUPER >(), CLASSID, sizeof(OUTER::CLASS), 0); \
	\
	const traktor::Type& OUTER::CLASS::getClassType() { \
		return m__type__; \
	} \
	const traktor::Type& OUTER::CLASS::getType() const { \
		return m__type__; \
	}

struct T_DLLCLASS ObjectFactory
{
	virtual bool isEditable() const = 0;

	virtual T_GC_NEW_TYPE(Object) newInstance() const = 0;
};

/*! \brief RTTI type descriptor. */
class T_DLLCLASS Type
{
public:
	Type(const Type* super, const wchar_t* name, size_t size, const ObjectFactory* factory);

	virtual ~Type();

	/*! \brief Return super type.
	 *
	 * \return Super type.
	 */
	const Type* getSuper() const;

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

	/*! \brief Instantiable type.
	 *
	 * \return True if type is instantiable.
	 */
	bool isInstantiable() const;

	/*! \brief Editable type.
	 *
	 * \return True if type is editable.
	 */
	bool isEditable() const;

	/*! \brief Create new instance of type.
	 *
	 * \return New instance.
	 */
	T_GC_NEW_TYPE(Object) newInstance() const;

	/*! \brief Find type from string representation.
	 *
	 * \return Type pointer, null if type not found.
	 */
	static const Type* find(const std::wstring& name);

	/*! \brief Find all types derived from this type.
	 *
	 * \param outTypes Found types.
	 * \param inclusive If this type should be included in result.
	 */
	void findAllOf(std::vector< const Type* >& outTypes, bool inclusive = true) const;

private:
	const Type* m_super;
	const wchar_t* m_name;
	size_t m_size;
	const ObjectFactory* m_factory;
};

typedef std::set< const Type* > TypeSet;

/*! \brief Get type of class */
template < typename T >
inline const Type& type_of()
{
	return T::getClassType();
}

/*! \brief Check if type is identical. */
inline bool is_type_a(const Type& base, const Type& type)
{
	return &base == &type;
}

/*! \brief Check if type is identical. */
template < typename T >
inline bool is_type_a(const Type& type)
{
	return is_type_a(type_of< T >(), type);
}

/*! \brief Check if type is derived from a base type. */
inline bool is_type_of(const Type& base, const Type& type)
{
	if (is_type_a(base, type))
		return true;

	if (!type.getSuper())
		return false;

	return is_type_of(base, *type.getSuper());
}

/*! \brief Check if type is derived from a base type. */
template < typename T >
inline bool is_type_of(const Type& type)
{
	return is_type_of(type_of< T >(), type);
}

/*! \brief Return type difference. */
inline uint32_t type_difference(const Type& base, const Type& type)
{
	uint32_t difference = 0;

	// Traverse up in inheritance chain from until we reach base type.
	for (const Type* i = &type; i; i = i->getSuper(), ++difference)
	{
		if (i == &base)
			return difference;
	}

	// Unable to reach base type; add inheritance depth of base type.
	for (const Type* i = &base; i; i = i->getSuper())
		++difference;

	return difference;
}

/*! \brief Return type difference. */
template < typename T >
inline uint32_t type_difference(const Type& type)
{
	return type_difference(type_of< T >(), type);
}

//@}

}

#endif	// traktor_kernel_Type_H
