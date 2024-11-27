/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/StaticMap.h"
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

/*! Serializer common class.
 * \ingroup Core
 */
class T_DLLCLASS Serializer : public ISerializer
{
	T_RTTI_CLASS;

public:
	typedef StaticMap< const TypeInfo*, int32_t, 16 > dataVersionMap_t;

	Ref< ISerializable > readObject();

	bool writeObject(const ISerializable* o);

	virtual int32_t getVersion() const override;

	virtual int32_t getVersion(const TypeInfo& typeInfo) const override;

	virtual void failure() override;

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
		int32_t v = 0;
		const dataVersionMap_t* dvm = nullptr;
	};

	AlignedVector< Version > m_versions;
	uint32_t m_versionPointer = 0;
	bool m_failure = false;
};

}

