/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_hf_HeightfieldResource_H
#define traktor_hf_HeightfieldResource_H

#include <string>
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class T_DLLCLASS HeightfieldResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	HeightfieldResource();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const Vector4& getWorldExtent() const { return m_worldExtent; }

private:
	friend class HeightfieldPipeline;

	Vector4 m_worldExtent;
};

	}
}

#endif	// traktor_hf_HeightfieldResource_H
