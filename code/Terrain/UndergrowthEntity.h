#ifndef traktor_terrain_UndergrowthEntity_H
#define traktor_terrain_UndergrowthEntity_H

#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/Half.h"
#include "Core/Thread/Job.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class MaterialMask;

	}

	namespace render
	{

class RenderContext;
class VertexBuffer;
class IndexBuffer;
class Shader;

	}

	namespace world
	{

class IWorldRenderPass;
class WorldRenderView;

	}

	namespace terrain
	{

class Terrain;

class T_DLLCLASS UndergrowthEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	enum { InstanceCount = 180 };

	struct Settings
	{
		int32_t density;
		float spreadDistance;
		float cellRadius;
		float plantScale;

		Settings()
		:	density(1000)
		,	spreadDistance(30.0f)
		,	cellRadius(20.0f)
		,	plantScale(1.0f)
		{
		}
	};

#pragma pack(1)
	struct Vertex
	{
		float position[3];
		half_t texCoord[2];
	};
#pragma pack()

	UndergrowthEntity(
		const resource::Proxy< Terrain >& terrain,
		const resource::Proxy< hf::MaterialMask >& materialMask,
		const Settings& settings,
		render::VertexBuffer* vertexBuffer,
		render::IndexBuffer* indexBuffer,
		const resource::Proxy< render::Shader >& shader
	);

	virtual ~UndergrowthEntity();

	void render(
		render::RenderContext* renderContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

	virtual Aabb3 getBoundingBox() const;

	virtual void update(const world::UpdateParams& update);

private:
	struct Cluster
	{
		Vector4 center;
		float distance;
		uint8_t plant;
		bool visible;
		int32_t from;
		int32_t to;
	};

	resource::Proxy< Terrain > m_terrain;
	resource::Proxy< hf::MaterialMask > m_materialMask;
	Settings m_settings;
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	resource::Proxy< render::Shader > m_shader;
	AlignedVector< Cluster > m_clusters;
	AlignedVector< Vector4 > m_plants;
	Vector4 m_eye;
};

	}
}

#endif	// traktor_terrain_UndergrowthEntity_H
