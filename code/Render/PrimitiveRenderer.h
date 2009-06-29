#ifndef traktor_render_PrimitiveRenderer_H
#define traktor_render_PrimitiveRenderer_H

#include <stack>
#include "Core/Math/Color.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Aabb.h"
#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class RenderSystem;
class RenderView;
class Shader;
class VertexBuffer;

/*! \brief Primitive renderer.
 * \ingroup Render
 *
 * The primitive renderer is a helper class to render
 * very simple primitives such as lines, boxes etc.
 * It's primary use is for the editor in order to render
 * editor hints and such.
 * It's NOT designed with performance in mind and should
 * NOT be used in time critical rendering.
 */
class T_DLLCLASS PrimitiveRenderer : public Object
{
	T_RTTI_CLASS(PrimitiveRenderer)

public:
	PrimitiveRenderer();

	bool create(
		resource::IResourceManager* resourceManager,
		RenderSystem* renderSystem
	);

	bool create(
		resource::IResourceManager* resourceManager,
		RenderSystem* renderSystem,
		const resource::Proxy< Shader >& shader
	);

	void destroy();

	void pushProjection(const Matrix44& projection);

	void popProjection();

	void pushView(const Matrix44& view);

	void popView();

	void pushWorld(const Matrix44& transform);

	void popWorld();

	void setClipDistance(float nearZ);

	void drawLine(
		const Vector4& start,
		const Vector4& end,
		const Color& color
	);

	void drawLine(
		const Vector4& start,
		const Vector4& end,
		float width,
		const Color& color
	);

	void drawWireAabb(
		const Vector4& center,
		const Vector4& extent,
		const Color& color
	);

	void drawWireAabb(
		const Aabb& aabb,
		const Color& color
	);

	void drawWireTriangle(
		const Vector4& vert1,
		const Vector4& vert2,
		const Vector4& vert3,
		const Color& color
	);

	void drawWireTriangle(
		const Vector4& vert1,
		const Color& color1,
		const Vector4& vert2,
		const Color& color2,
		const Vector4& vert3,
		const Color& color3
	);

	void drawWireQuad(
		const Vector4& vert1,
		const Vector4& vert2,
		const Vector4& vert3,
		const Vector4& vert4,
		const Color& color
	);

	void drawWireQuad(
		const Vector4& vert1,
		const Color& color1,
		const Vector4& vert2,
		const Color& color2,
		const Vector4& vert3,
		const Color& color3,
		const Vector4& vert4,
		const Color& color4
	);

	void drawWireCylinder(
		const Matrix44& frame,
		float radius,
		float length,
		const Color& color
	);

	void drawSolidAabb(
		const Vector4& center,
		const Vector4& extent,
		const Color& color
	);

	void drawSolidAabb(
		const Aabb& aabb,
		const Color& color
	);

	void drawSolidTriangle(
		const Vector4& vert1,
		const Vector4& vert2,
		const Vector4& vert3,
		const Color& color
	);

	void drawSolidTriangle(
		const Vector4& vert1,
		const Color& color1,
		const Vector4& vert2,
		const Color& color2,
		const Vector4& vert3,
		const Color& color3
	);

	void drawSolidQuad(
		const Vector4& vert1,
		const Vector4& vert2,
		const Vector4& vert3,
		const Vector4& vert4,
		const Color& color
	);

	void drawSolidQuad(
		const Vector4& vert1,
		const Color& color1,
		const Vector4& vert2,
		const Color& color2,
		const Vector4& vert3,
		const Color& color3,
		const Vector4& vert4,
		const Color& color4
	);

	void drawProtractor(
		const Vector4& position,
		const Vector4& base,
		const Vector4& zero,
		float minAngle,
		float maxAngle,
		float angleStep,
		float radius,
		const Color& colorSolid,
		const Color& colorHint
	);

	void drawCone(
		const Matrix44& frame,
		float angleX,
		float angleY,
		float length,
		const Color& colorSolid,
		const Color& colorHint
	);

	bool begin(RenderView* renderView);

	void end(RenderView* renderView);

	inline const Matrix44& getProjection() const { return m_projection.back(); }

	inline const Matrix44& getView() const { return m_view.back(); }

	inline const Matrix44& getWorld() const { return m_world.back(); }

private:
	resource::Proxy< Shader > m_shader;
	Ref< VertexBuffer > m_vertexBuffers[3];
	int m_currentBuffer;
	struct Vertex* m_vertexStart;
	struct Vertex* m_vertex;
	std::vector< Primitives > m_primitives;
	AlignedVector< Matrix44 > m_projection;
	AlignedVector< Matrix44 > m_view;
	AlignedVector< Matrix44 > m_world;
	Matrix44 m_worldView;
	Matrix44 m_worldViewProj;
	float m_viewNearZ;
	float m_viewWidth;
	float m_viewHeight;

	void updateTransforms();
};

	}
}

#endif	// traktor_render_PrimitiveRenderer_H
