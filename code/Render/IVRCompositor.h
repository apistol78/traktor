/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IVRCompositor_H
#define traktor_render_IVRCompositor_H

#include "Core/Object.h"

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

class IRenderSystem;
class IRenderView;

/*! \brief VR compositor interface.
 * \ingroup Render
 *
 * This class provide an interface for the engine
 * to handle integration of any third-party
 * VR SDK/API.
 *
 * Engine access this interface by calling
 * 1. beginRenderEye(left)
 * 2. beginRenderEye(right)
 * 3. presentCompositeOutput()
 * for each frame.
 */
class T_DLLCLASS IVRCompositor : public Object
{
	T_RTTI_CLASS;
	
public:
	/*! \brief Create VR compositor.
	 *
	 * Create the VR compositor and implement
	 * the connection to the underlying VR subsystem.
	 *
	 * As render system and view is already created
	 * the VR compositor must provide the glue between
	 * those and the VR subsystem.
	 *
	 * \param renderSystem Render system implementation.
	 * \param renderView Render view implementation.
	 */
	virtual bool create(IRenderSystem* renderSystem, IRenderView* renderView) = 0;

	/*! \brief Destroy VR compositor.
	 */
	virtual void destroy() = 0;

	/*! \brief Get width of renderable output.
	 *
	 * As VR compositor might require different
	 * size of the renderable view the application
	 * query this method to get the required output width.
	 *
	 * \return Render output width.
	 */
	virtual int32_t getWidth() const = 0;

	/*! \brief Get height of renderable output.
	 *
	 * As VR compositor might require different
	 * size of the renderable view the application
	 * query this method to get the required output height.
	 *
	 * \return Render output height.
	 */
	virtual int32_t getHeight() const = 0;

	/*! \brief Begin rendering of either left or right eye.
	 *
	 * This method should be called before rendering
	 * of either eye to ensure the VR compositor is ready
	 * to consume rendered content.
	 *
	 * \param renderView Render view implementation.
	 * \param eye Which eye should be rendered.
	 * \return True if rendering should be performed.
	 */
	virtual bool beginRenderEye(IRenderView* renderView, int32_t eye) = 0;

	/*! \brief End rendering of either left or right eye.
	 *
	 * \param renderView Render view implementation.
	 * \param eye Which eye should be rendered.
	 * \return True if rendering should be performed.
	 */
	virtual bool endRenderEye(IRenderView* renderView, int32_t eye) = 0;

	/*! \brief Perform final composition of VR output.
	 *
	 * This method assume both eyes has been rendered
	 * so the compositor can provide required frames to
	 * VR subsystem.
	 *
	 * \param renderView Render view implementation.
	 * \return True if rendering should be performed.
	 */
	virtual bool presentCompositeOutput(IRenderView* renderView) = 0;	
};

	}
}

#endif	// traktor_render_IVRCompositor_H
