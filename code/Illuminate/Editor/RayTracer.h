#pragma once

#include "Core/Object.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Illuminate/Editor/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ILLUMINATE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

class Model;

	}

	namespace illuminate
	{

class T_DLLCLASS RayTracer : public Object
{
	T_RTTI_CLASS;

public:
	RayTracer();

	void addLight(const Light& light);

	void addModel(const model::Model* model, const Transform& transform);

	bool prepare();

	Color4f traceDirect(const Vector4& origin, const Vector4& normal, float roughness);

	Color4f traceIndirect(const Vector4& origin, const Vector4& normal, float roughness);

private:
	SahTree m_sah;
	SahTree::QueryCache m_sahCache;
	AlignedVector< Light > m_lights;
	AlignedVector< Winding3 > m_windings;
	RandomGeometry m_random;
	uint32_t m_irradianceSampleCount;
	float m_maxDistance;

	Color4f sampleAnalyticalLights(const Vector4& origin, const Vector4& normal);
};

	}
}
