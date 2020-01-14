#pragma once

#include <vector>
#include "Render/Types.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "Render/Image/ImageProcessStep.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;
class VertexBuffer;

/*! \brief
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepLensDirt : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	struct Source
	{
		std::wstring param;		/*!< Shader parameter name. */
		std::wstring source;	/*!< Render target set source. */

		Source();

		void serialize(ISerializer& s);
	};

	class InstanceLensDirt : public Instance
	{
	public:
		enum { InstanceCount = 64 };

		struct Source
		{
			handle_t param;
			handle_t source;
		};

		InstanceLensDirt(const ImageProcessStepLensDirt* step);

		virtual void destroy() override final;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) override final;

	private:
		friend class ImageProcessStepLensDirt;

		Ref< const ImageProcessStepLensDirt > m_step;
		resource::Proxy< Shader > m_shader;
		std::vector< Source > m_sources;
		Ref< VertexBuffer > m_vertexBuffer;
		Primitives m_primitives;
		handle_t m_handleInstances;
		Vector4 m_instances[InstanceCount];
	};

	ImageProcessStepLensDirt();

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< Shader >& getShader() const { return m_shader; }

	const std::vector< Source >& getSources() const { return m_sources; }

private:
	resource::Id< Shader > m_shader;
	std::vector< Source > m_sources;
};

	}
}

