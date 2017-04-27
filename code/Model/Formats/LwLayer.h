/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_LwLayer_H
#define traktor_model_LwLayer_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Model/Formats/LwPolygon.h"

namespace traktor
{

class IStream;

	namespace model
	{

struct LwChunk;
class LwVMad;
class LwVMap;

class LwLayer : public Object
{
	T_RTTI_CLASS;

public:
	LwLayer();

	bool read(const LwChunk& chunk, IStream* stream);

	const AlignedVector< Vector4 >& getPoints() const { return m_points; }

	const AlignedVector< LwPolygon >& getPolygons() const { return m_polygons; }

	const RefArray< LwVMap >& getValueMaps() const { return m_vmaps; }

	const RefArray< LwVMad >& getDiscontinousValueMaps() const { return m_vmads; }

private:
	friend class LwObject;

	uint16_t m_index;
	uint16_t m_flags;
	Vector4 m_pivot;
	std::wstring m_name;
	uint16_t m_parent;
	AlignedVector< Vector4 > m_points;
	AlignedVector< LwPolygon > m_polygons;
	RefArray< LwVMap > m_vmaps;
	RefArray< LwVMad > m_vmads;
};

	}
}

#endif	// traktor_model_LwLayer_H
