#ifndef traktor_mesh_MeshEntity_H
#define traktor_mesh_MeshEntity_H

#include "World/Entity/SpatialEntity.h"
#include "Core/Math/Aabb.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class WorldContext;
class WorldRenderView;

	}

	namespace mesh
	{

class IMeshParameterCallback;

/*! \brief Base mesh entity.
 *
 * Spatial mesh entity class.
 * Each mesh entity currently support a single user defined
 * parameter to be passed to the shader system when
 * the mesh is rendered.
 */
class T_DLLCLASS MeshEntity : public world::SpatialEntity
{
	T_RTTI_CLASS(MeshEntity)

public:
	MeshEntity(const Matrix44& transform);

	virtual void setTransform(const Matrix44& transform);

	virtual bool getTransform(Matrix44& outTransform) const;
	
	virtual void render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance) = 0;

	inline void setUserParameter(const float userParameter) { m_userParameter = userParameter; }

	inline float getUserParameter() const { return m_userParameter; }

	inline void setParameterCallback(IMeshParameterCallback* parameterCallback) { m_parameterCallback = parameterCallback; }

	inline IMeshParameterCallback* getParameterCallback() const { return m_parameterCallback; }

protected:
	Matrix44 m_transform;
	float m_userParameter;
	IMeshParameterCallback* m_parameterCallback;
};

	}
}

#endif	// traktor_mesh_MeshEntity_H
