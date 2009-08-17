#ifndef traktor_Serializable_H
#define traktor_Serializable_H

#include "Core/Object.h"
#include "Core/Heap/GcNew.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \ingroup */
//@{

class Serializer;

/*! \brief Base class of each serializable class.
 *
 * Any class supporting serialization must be
 * derived from this class.
 */
class T_DLLCLASS Serializable : public Object
{
	T_RTTI_CLASS(Serializable)

public:
	/*! \brief Return serialization version.
	 *
	 * In order to support several generations
	 * of serialized data this method can be overridden
	 * to handle multiple versions of the same serializable.
	 * When de-serializing an older version it's required to
	 * use the Serializer::getVersion method to determine
	 * which members to actually serialize.
	 * /note Even members which aren't used anymore must be de-serialized properly.
	 */
	virtual int getVersion() const;

	/*! \brief Serialize object.
	 *
	 * \param s Serializer interface.
	 * \return True if serialization successful.
	 */
	virtual bool serialize(Serializer& s) = 0;
};

/*! \brief Generic object factory. */
template < typename ClassType, bool Editable >
struct DefaultFactory : public ObjectFactory
{
	virtual bool isEditable() const {
		return Editable;
	}

	virtual Object* newInstance() const {
		return gc_new< ClassType >();
	}
};

#define T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(CLASSID, CLASS, SUPER) \
	const traktor::Type CLASS::m__type__(&traktor::type_of< SUPER >(), CLASSID, sizeof(CLASS), new traktor::DefaultFactory< CLASS, false >); \
	\
	const traktor::Type& CLASS::getClassType() { \
		return m__type__; \
	} \
	const traktor::Type& CLASS::getType() const { \
		return m__type__; \
	}

#define T_IMPLEMENT_RTTI_SERIALIZABLE_COMPOSITE_CLASS(CLASSID, OUTER, CLASS, SUPER) \
	const traktor::Type OUTER::CLASS::m__type__(&traktor::type_of< SUPER >(), CLASSID, sizeof(OUTER::CLASS), new traktor::DefaultFactory< OUTER::CLASS, false >); \
	\
	const traktor::Type& OUTER::CLASS::getClassType() { \
		return m__type__; \
	} \
	const traktor::Type& OUTER::CLASS::getType() const { \
		return m__type__; \
	}

#define T_IMPLEMENT_RTTI_EDITABLE_CLASS(CLASSID, CLASS, SUPER) \
	const traktor::Type CLASS::m__type__(&traktor::type_of< SUPER >(), CLASSID, sizeof(CLASS), new traktor::DefaultFactory< CLASS, true >); \
	\
	const traktor::Type& CLASS::getClassType() { \
		return m__type__; \
	} \
	const traktor::Type& CLASS::getType() const { \
		return m__type__; \
	}

#define T_IMPLEMENT_RTTI_EDITABLE_COMPOSITE_CLASS(CLASSID, OUTER, CLASS, SUPER) \
	const traktor::Type OUTER::CLASS::m__type__(&traktor::type_of< SUPER >(), CLASSID, sizeof(OUTER::CLASS), new traktor::DefaultFactory< OUTER::CLASS, true >); \
	\
	const traktor::Type& OUTER::CLASS::getClassType() { \
		return m__type__; \
	} \
	const traktor::Type& OUTER::CLASS::getType() const { \
		return m__type__; \
	}

//! }

}

#endif	// traktor_Serializable_H
