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

class IlluminateConfiguration;

class T_DLLCLASS RayTracer : public Object
{
	T_RTTI_CLASS;

public:
	struct Context : public RefCountImpl< IRefCount >
	{
		SahTree::QueryCache sahCache;
		RandomGeometry random;
		AlignedVector< Vector4 > dirs;
	};

	struct Result
	{
		Scalar distance;
		Vector4 position;
		Vector4 normal;
	};

	explicit RayTracer(const IlluminateConfiguration* configuration);

	void addLight(const Light& light);

	void addModel(const model::Model* model, const Transform& transform);

	bool prepare();

	Ref< Context > createContext();

	bool trace(Context* context, const Vector4& origin, const Vector4& direction, const Scalar& maxDistance, Result& outResult) const;

	Color4f traceDirect(Context* context, const Vector4& origin, const Vector4& normal) const;

	Color4f traceIndirect(Context* context, const Vector4& origin, const Vector4& normal) const;

private:
	struct Surface
	{
		Color4f albedo;
	};

	const IlluminateConfiguration* m_configuration;
	SahTree m_sah;
	AlignedVector< Light > m_lights;
	AlignedVector< Winding3 > m_windings;
	AlignedVector< Surface > m_surfaces;
	float m_maxDistance;

	Color4f sampleAnalyticalLights(Context* context, const Vector4& origin, const Vector4& normal, uint32_t shadowSampleCount, float pointLightShadowRadius) const;
};

	}
}
