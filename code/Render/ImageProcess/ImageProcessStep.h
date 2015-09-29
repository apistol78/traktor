#ifndef traktor_render_ImageProcessStep_H
#define traktor_render_ImageProcessStep_H

#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class ImageProcess;
class IRenderSystem;
class IRenderView;
class ScreenRenderer;

/*! \brief Post processing step.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStep : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! \brief Step instance. */
	class T_DLLCLASS Instance : public Object
	{
	public:
		struct RenderParams
		{
			Frustum viewFrustum;
			Matrix44 view;
			Matrix44 viewToLight;
			Matrix44 projection;
			Vector4 godRayDirection;
			int32_t sliceCount;
			int32_t sliceIndex;
			float sliceNearZ;
			float sliceFarZ;
			float shadowFarZ;
			float shadowMapBias;
			float deltaTime;
		};

		virtual void destroy() = 0;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) = 0;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const = 0;
};

	}
}

#endif	// traktor_render_ImageProcessStep_H
