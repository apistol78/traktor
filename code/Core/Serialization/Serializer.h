#ifndef traktor_Serializer_H
#define traktor_Serializer_H

#include <list>
#include "Core/Ref.h"
#include "Core/Serialization/ISerializer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Serializer common class.
 * \ingroup Core
 */
class T_DLLCLASS Serializer : public ISerializer
{
	T_RTTI_CLASS;

public:
	Ref< ISerializable > readObject();

	bool writeObject(const ISerializable* o);

	virtual int getVersion() const;

	virtual ISerializable* getCurrentObject();

	virtual ISerializable* getOuterObject();

	template < typename T >
	Ref< T > readObject()
	{
		Ref< ISerializable > object = readObject();
		return dynamic_type_cast< T* >(object);
	}

protected:
	bool serialize(ISerializable* inner, int version, ISerializable* outer);

private:
	std::list< std::pair< ISerializable*, int > > m_constructing;
};

}

#endif	// traktor_Serializer_H
