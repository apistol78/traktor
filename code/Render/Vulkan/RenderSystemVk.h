/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderSystemVk_H
#define traktor_render_RenderSystemVk_H

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__LINUX__)
#	define VK_USE_PLATFORM_LINUX_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#endif

#include "Core/Ref.h"
#include "Render/IRenderSystem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_VULKAN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextVk;

#if defined(_WIN32) || defined(__LINUX__)
class Window;
#endif

/*! \brief Vulkan render system.
 * \ingroup Vulkan
 *
 * Vulkan render system implementation.
 */
class T_DLLCLASS RenderSystemVk : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemVk();

	virtual bool create(const RenderSystemDesc& desc) override final;

	virtual void destroy() override final;

	virtual bool reset(const RenderSystemDesc& desc) override final;

	virtual void getInformation(RenderSystemInformation& outInfo) const override final;

	virtual uint32_t getDisplayModeCount() const override final;
	
	virtual DisplayMode getDisplayMode(uint32_t index) const override final;
	
	virtual DisplayMode getCurrentDisplayMode() const override final;

	virtual float getDisplayAspectRatio() const override final;

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc) override final;

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc) override final;

	virtual Ref< VertexBuffer > createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic) override final;

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic) override final;

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc) override final;

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc) override final;
	
	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc) override final;

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc) override final;

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource, const wchar_t* const tag) override final;

	virtual Ref< ITimeQuery > createTimeQuery() const override final;

	virtual void purge() override final;

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const override final;

private:
#if defined(_WIN32) || defined(__LINUX__)
	Ref< Window > m_window;
#endif
	VkInstance m_instance;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physicalDevice;
	uint32_t m_physicalDeviceQueueIndex;
	VkDevice m_device;
	bool m_haveValidationLayer;
};

	}
}

#endif	// traktor_render_RenderSystemVk_H
