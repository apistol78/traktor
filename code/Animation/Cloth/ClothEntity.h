#ifndef traktor_animation_ClothEntity_H
#define traktor_animation_ClothEntity_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector2.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Resource/Proxy.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

	}

	namespace animation
	{

class T_DLLCLASS ClothEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	struct Node
	{
		Vector4 position[2];
		Vector2 texCoord;
		Scalar invMass;
	};

	struct Edge
	{
		uint32_t index[2];
		Scalar length;
	};

	ClothEntity();

	virtual ~ClothEntity();

	bool create(
		render::IRenderSystem* renderSystem,
		const resource::Proxy< render::Shader >& shader,
		uint32_t resolutionX,
		uint32_t resolutionY,
		float scale,
		float damping,
		uint32_t solverIterations
	);

	void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	void reset();

	void setNodeInvMass(uint32_t x, uint32_t y, float invMass);

	const AlignedVector< Node >& getNodes() const { return m_nodes; }

	const AlignedVector< Edge >& getEdges() const { return m_edges; }

private:
	AlignedVector< Node > m_nodes;
	AlignedVector< Edge > m_edges;
	Transform m_transform;
	float m_time;
	float m_updateTime;
	float m_scale;
	Scalar m_damping;
	uint32_t m_solverIterations;
	uint32_t m_resolutionX;
	uint32_t m_resolutionY;
	uint32_t m_triangleCount;
	traktor::Aabb3 m_aabb;
	bool m_updateRequired;
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	resource::Proxy< render::Shader > m_shader;
};

	}
}

#endif	// traktor_animation_ClothEntity_H
