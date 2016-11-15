#ifndef traktor_render_PrimitiveRenderer_H
#define traktor_render_PrimitiveRenderer_H

#include <stack>
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Types.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"

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

class IRenderSystem;
class IRenderView;
class ITexture;
class Shader;
class VertexBuffer;

struct Vertex;

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
	T_RTTI_CLASS;

public:
	PrimitiveRenderer();

	bool create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t frameCount
	);

	bool create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		const resource::Id< Shader >& shader,
		uint32_t frameCount
	);

	void destroy();

	bool begin(uint32_t frame, const Matrix44& projection);

	void end(uint32_t frame);

	void render(IRenderView* renderView, uint32_t frame);

	void pushView(const Matrix44& view);

	void popView();

	void pushWorld(const Matrix44& transform);

	void popWorld();

	void pushDepthState(bool depthTest, bool depthWrite, bool depthOutput);

	void popDepthState();

	void setProjection(const Matrix44& projection);

	void setClipDistance(float nearZ);

	void drawLine(
		const Vector4& start,
		const Vector4& end,
		const Color4ub& color
	);

	void drawLine(
		const Vector4& start,
		const Vector4& end,
		float width,
		const Color4ub& color
	);

	void drawArrowHead(
		const Vector4& start,
		const Vector4& end,
		float sharpness,
		const Color4ub& color
	);

	void drawWireAabb(
		const Vector4& center,
		const Vector4& extent,
		const Color4ub& color
	);

	void drawWireAabb(
		const Aabb3& aabb,
		const Color4ub& color
	);

	void drawWireTriangle(
		const Vector4& vert1,
		const Vector4& vert2,
		const Vector4& vert3,
		const Color4ub& color
	);

	void drawWireTriangle(
		const Vector4& vert1,
		const Color4ub& color1,
		const Vector4& vert2,
		const Color4ub& color2,
		const Vector4& vert3,
		const Color4ub& color3
	);

	void drawWireQuad(
		const Vector4& vert1,
		const Vector4& vert2,
		const Vector4& vert3,
		const Vector4& vert4,
		const Color4ub& color
	);

	void drawWireQuad(
		const Vector4& vert1,
		const Color4ub& color1,
		const Vector4& vert2,
		const Color4ub& color2,
		const Vector4& vert3,
		const Color4ub& color3,
		const Vector4& vert4,
		const Color4ub& color4
	);

	void drawWireCircle(
		const Vector4& center,
		const Vector4& normal,
		float radius,
		float width,
		const Color4ub& color
	);

	void drawWireSphere(
		const Matrix44& center,
		float radius,
		const Color4ub& color
	);

	void drawWireCylinder(
		const Matrix44& frame,
		float radius,
		float length,
		const Color4ub& color
	);

	void drawWireFrame(
		const Matrix44& frame,
		float length
	);

	void drawSolidPoint(
		const Vector4& center,
		float size,
		const Color4ub& color
	);

	void drawSolidAabb(
		const Vector4& center,
		const Vector4& extent,
		const Color4ub& color
	);

	void drawSolidAabb(
		const Aabb3& aabb,
		const Color4ub& color
	);

	void drawSolidTriangle(
		const Vector4& vert1,
		const Vector4& vert2,
		const Vector4& vert3,
		const Color4ub& color
	);

	void drawSolidTriangle(
		const Vector4& vert1,
		const Color4ub& color1,
		const Vector4& vert2,
		const Color4ub& color2,
		const Vector4& vert3,
		const Color4ub& color3
	);

	void drawSolidQuad(
		const Vector4& vert1,
		const Vector4& vert2,
		const Vector4& vert3,
		const Vector4& vert4,
		const Color4ub& color
	);

	void drawSolidQuad(
		const Vector4& vert1,
		const Color4ub& color1,
		const Vector4& vert2,
		const Color4ub& color2,
		const Vector4& vert3,
		const Color4ub& color3,
		const Vector4& vert4,
		const Color4ub& color4
	);

	void drawTextureTriangle(
		const Vector4& vert1,
		const Vector2& texCoord1,
		const Vector4& vert2,
		const Vector2& texCoord2,
		const Vector4& vert3,
		const Vector2& texCoord3,
		const Color4ub& color,
		ITexture* texture
	);

	void drawTextureQuad(
		const Vector4& vert1,
		const Vector2& texCoord1,
		const Vector4& vert2,
		const Vector2& texCoord2,
		const Vector4& vert3,
		const Vector2& texCoord3,
		const Vector4& vert4,
		const Vector2& texCoord4,
		const Color4ub& color,
		ITexture* texture
	);

	void drawProtractor(
		const Vector4& position,
		const Vector4& base,
		const Vector4& zero,
		float minAngle,
		float maxAngle,
		float angleStep,
		float radius,
		const Color4ub& colorSolid,
		const Color4ub& colorHint
	);

	void drawCone(
		const Matrix44& frame,
		float angleX,
		float angleY,
		float length,
		const Color4ub& colorSolid,
		const Color4ub& colorHint
	);

	const Matrix44& getProjection() const { return m_currentFrame->projections.back(); }

	const Matrix44& getView() const { return m_view.back(); }

	const Matrix44& getWorld() const { return m_world.back(); }

private:
	struct DepthState
	{
		bool depthTest;
		bool depthWrite;
		bool depthOutput;

		DepthState()
		:	depthTest(false)
		,	depthWrite(false)
		,	depthOutput(false)
		{
		}

		DepthState(bool _depthTest, bool _depthWrite, bool _depthOutput)
		:	depthTest(_depthTest)
		,	depthWrite(_depthWrite)
		,	depthOutput(_depthOutput)
		{
		}

		bool operator == (const DepthState& rh) const
		{
			return depthTest == rh.depthTest && depthWrite == rh.depthWrite && depthOutput == rh.depthOutput;
		}

		bool operator != (const DepthState& rh) const
		{
			return !(*this == rh);
		}
	};

	struct Batch
	{
		uint32_t projection;
		DepthState depthState;
		Ref< VertexBuffer > vertexBuffer;
		Ref< ITexture > texture;
		Primitives primitives;
	};

	struct Frame
	{
		RefArray< VertexBuffer > vertexBuffers;
		AlignedVector< Matrix44 > projections;
		AlignedVector< Batch > batches;
	};

	// System
	Ref< IRenderSystem > m_renderSystem;
	resource::Proxy< Shader > m_shader;
	RefArray< VertexBuffer > m_freeVertexBuffers;
	Semaphore m_lock;

	// Frame
	AlignedVector< Frame > m_frames;
	Frame* m_currentFrame;

	// Assembly state.
	AlignedVector< Matrix44 > m_view;
	AlignedVector< Matrix44 > m_world;
	AlignedVector< DepthState > m_depthState;
	Matrix44 m_worldView;
	float m_viewNearZ;
	Vertex* m_vertexHead;
	Vertex* m_vertexTail;

	void updateTransforms();

	Vertex* allocBatch(render::PrimitiveType primitiveType, uint32_t primitiveCount, render::ITexture* texture);
};

	}
}

#endif	// traktor_render_PrimitiveRenderer_H
