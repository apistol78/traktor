/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Serializer_H
#define traktor_Serializer_H

#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
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
	typedef SmallMap< const TypeInfo*, int32_t > dataVersionMap_t;

	Serializer();

	Ref< ISerializable > readObject();

	bool writeObject(const ISerializable* o);

	virtual int32_t getVersion() const T_OVERRIDE;

	virtual int32_t getVersion(const TypeInfo& typeInfo) const T_OVERRIDE;

	virtual void failure() T_OVERRIDE;

	template < typename T >
	Ref< T > readObject()
	{
		Ref< ISerializable > object = readObject();
		return dynamic_type_cast< T* >(object);
	}

protected:
	void serialize(ISerializable* inner);

	void serialize(ISerializable* inner, const dataVersionMap_t& dataVersions);

	bool failed() const { return m_failure; }

private:
	struct Version
	{
		int32_t v;
		dataVersionMap_t dvm;
	};

	AlignedVector< Version > m_versions;
	bool m_failure;
};

}

#endif	// traktor_Serializer_H
