#ifndef traktor_render_ImageProcessStepTemporal_H
#define traktor_render_ImageProcessStepTemporal_H

#include <vector>
#include "Render/Types.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "Render/ImageProcess/ImageProcessStep.h"

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

/*! \brief Step useful for temporal filtering.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepTemporal : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	struct Source
	{
		std::wstring param;		/*!< Shader parameter name. */
		std::wstring source;	/*!< Render target set source. */
		uint32_t index;			/*!< Render target index. */

		Source();

		void serialize(ISerializer& s);
	};

	class InstanceTemporal : public Instance
	{
	public:
		struct Source
		{
			handle_t param;
			handle_t source;
			uint32_t index;
		};

		InstanceTemporal(const ImageProcessStepTemporal* step, const resource::Proxy< Shader >& shader, const std::vector< Source >& sources);

		virtual void destroy() T_OVERRIDE T_FINAL;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) T_OVERRIDE T_FINAL;

	private:
		Ref< const ImageProcessStepTemporal > m_step;
		resource::Proxy< Shader > m_shader;
		std::vector< Source > m_sources;
		float m_time;
		Matrix44 m_previousView;
		handle_t m_handleTime;
		handle_t m_handleDeltaTime;
		handle_t m_handleViewEdgeTopLeft;
		handle_t m_handleViewEdgeTopRight;
		handle_t m_handleViewEdgeBottomLeft;
		handle_t m_handleViewEdgeBottomRight;
		handle_t m_handleProjection;
		handle_t m_handleView;
		handle_t m_handleViewLast;
		handle_t m_handleViewInverse;
		handle_t m_handleDeltaView;
		handle_t m_handleDeltaViewProj;
		handle_t m_handleMagicCoeffs;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< Shader >& getShader() const { return m_shader; }

	const std::vector< Source >& getSources() const { return m_sources; }

private:
	resource::Id< Shader > m_shader;
	std::vector< Source > m_sources;
};

	}
}

#endif	// traktor_render_ImageProcessStepTemporal_H
