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
	Serializer();

	Ref< ISerializable > readObject();

	bool writeObject(const ISerializable* o);

	virtual int getVersion() const T_OVERRIDE;

	virtual ISerializable* getCurrentObject() T_OVERRIDE;

	virtual void failure() T_OVERRIDE;

	template < typename T >
	Ref< T > readObject()
	{
		Ref< ISerializable > object = readObject();
		return dynamic_type_cast< T* >(object);
	}

protected:
	void serialize(ISerializable* inner, int version);

	bool failed() const { return m_failure; }

private:
	std::list< std::pair< ISerializable*, int > > m_constructing;
	bool m_failure;
};

}

#endif	// traktor_Serializer_H
