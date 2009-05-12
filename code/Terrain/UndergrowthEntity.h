#ifndef traktor_terrain_UndergrowthEntity_H
#define traktor_terrain_UndergrowthEntity_H

#include "Resource/Proxy.h"
#include "World/Entity/Entity.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Random.h"
#include "Core/Math/Half.h"
#include "Core/Thread/JobManager.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class WorldRenderView;

	}

	namespace render
	{

class RenderContext;
class VertexBuffer;
class IndexBuffer;
class Shader;

	}

	namespace terrain
	{

class Heightfield;
class MaterialMask;

class T_DLLCLASS UndergrowthEntity : public world::Entity
{
	T_RTTI_CLASS(TerrainEntity)

public:
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
		half_t normal[4];
		half_t texCoord[2];

		void set(const Vector4& position, const Vector4& normal, float texCoordU, float texCoordV);
	};
#pragma pack()

	UndergrowthEntity(
		const resource::Proxy< Heightfield >& heightfield,
		const resource::Proxy< MaterialMask >& materialMask,
		const Settings& settings,
		render::VertexBuffer* vertexBuffer,
		render::IndexBuffer* indexBuffer,
		const render::Primitives& primitives,
		const resource::Proxy< render::Shader >& shader
	);

	virtual ~UndergrowthEntity();

	void render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView);

	virtual void update(const world::EntityUpdate* update);

private:
	struct Cell
	{
		Vector4 position;
		int offset;
		int count;
	};

	resource::Proxy< Heightfield > m_heightfield;
	resource::Proxy< MaterialMask > m_materialMask;
	Settings m_settings;
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	render::Primitives m_primitives;
	resource::Proxy< render::Shader > m_shader;
	Random m_random;
	Cell m_cells[16];
	Matrix44 m_lastView;
	Frustum m_lastFrustum;
	Job m_jobs[4];
	bool m_sync;

	void synchronize();

	void updateTask(int start, int end, Vertex* outVertex);
};

	}
}

#endif	// traktor_terrain_UndergrowthEntity_H
