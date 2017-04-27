/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_hf_HeightfieldFormat_H
#define traktor_hf_HeightfieldFormat_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Vector4.h"

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

class Heightfield;

class T_DLLCLASS HeightfieldFormat : public Object
{
	T_RTTI_CLASS;

public:
	Ref< Heightfield > read(IStream* stream, const Vector4& worldExtent) const;

	bool write(IStream* stream, const Heightfield* heightfield) const;
};

	}
}

#endif	// traktor_hf_HeightfieldFormat_H
