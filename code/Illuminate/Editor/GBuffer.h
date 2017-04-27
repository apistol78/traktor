/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_illuminate_GBuffer_H
#define traktor_illuminate_GBuffer_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/Path.h"
#include "Core/Math/Vector4.h"
#include "Illuminate/Editor/Types.h"

namespace traktor
{
	namespace illuminate
	{

class GBuffer : public Object
{
	T_RTTI_CLASS;

public:
	struct Element
	{
		int32_t surfaceIndex;
		Vector4 position;
		Vector4 normal;

		Element()
		:	surfaceIndex(-1)
		,	position(Vector4::zero())
		,	normal(Vector4::zero())
		{
		}
	};

	GBuffer();

	bool create(const AlignedVector< Surface >& surfaces, int32_t width, int32_t height);

	void dilate(int32_t iterations);

	void saveAsImages(const Path& outputPath) const;

	const Element& get(int32_t x, int32_t y) const { return m_data[x + y * m_width]; }

private:
	int32_t m_width;
	int32_t m_height;
	AlignedVector< Element > m_data;
};

	}
}

#endif	// traktor_illuminate_GBuffer_H
