#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/Boxes/BoxedPointer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ITexture.h"
#include "Render/ICubeTexture.h"
#include "Render/ISimpleTexture.h"
#include "Render/IVolumeTexture.h"
#include "Render/RenderClassFactory.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class BoxedDisplayMode : public Object
{
	T_RTTI_CLASS;

public:
	BoxedDisplayMode(const DisplayMode& displayMode)
	:	m_displayMode(displayMode)
	{
	}

	uint32_t getWidth() const { return m_displayMode.width; }

	uint32_t getHeight() const { return m_displayMode.height; }

	uint16_t getRefreshRate() const { return m_displayMode.refreshRate; }

	uint16_t getColorBits() const { return m_displayMode.colorBits; }

private:
	DisplayMode m_displayMode;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.DisplayMode", BoxedDisplayMode, Object)

class BoxedSimpleTextureCreateDesc : public Object
{
	T_RTTI_CLASS;

public:
	BoxedSimpleTextureCreateDesc()
	{
		// \fixme Expose texture formats to script.
		m_value.format = TfR8G8B8A8;
	}

	BoxedSimpleTextureCreateDesc(const SimpleTextureCreateDesc& stcd)
	:	m_value(stcd)
	{
	}

	void setWidth(int32_t width) { m_value.width = width; }

	int32_t getWidth() const { return m_value.width; }

	void setHeight(int32_t height) { m_value.height = height; }

	int32_t getHeight() const { return m_value.height; }

	void setMipCount(int32_t mipCount) { m_value.mipCount = mipCount; }

	int32_t getMipCount() const { return m_value.mipCount; }

	void set_sRGB(bool sRGB) { m_value.sRGB = sRGB; }

	bool get_sRGB() const { return m_value.sRGB; }

	void setImmutable(bool immutable) { m_value.immutable = immutable; }

	bool getImmutable() const { return m_value.immutable; }

	const SimpleTextureCreateDesc& unbox() const { return m_value; }

private:
	SimpleTextureCreateDesc m_value;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureCreateDesc", BoxedSimpleTextureCreateDesc, Object)

Ref< BoxedPointer > ISimpleTexture_lock(ISimpleTexture* self, int32_t level)
{
	ITexture::Lock lock;
	if (self->lock(level, lock))
		return new BoxedPointer(lock.bits);
	else
		return nullptr;
}

handle_t IRenderSystem_getHandle(const std::wstring& id)
{
	return getParameterHandle(id);
}

Ref< BoxedDisplayMode > IRenderSystem_getCurrentDisplayMode(IRenderSystem* this_)
{
	return new BoxedDisplayMode(this_->getCurrentDisplayMode());
}

Ref< BoxedDisplayMode > IRenderSystem_getDisplayMode(IRenderSystem* this_, uint32_t index)
{
	return new BoxedDisplayMode(this_->getDisplayMode(index));
}

Ref< ISimpleTexture > IRenderSystem_createSimpleTexture(IRenderSystem* self, const BoxedSimpleTextureCreateDesc* stcd)
{
	return self->createSimpleTexture(stcd->unbox(), T_FILE_LINE_W);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderClassFactory", 0, RenderClassFactory, IRuntimeClassFactory)

void RenderClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classBoxedDisplayMode = new AutoRuntimeClass< BoxedDisplayMode >();
	classBoxedDisplayMode->addProperty("width", &BoxedDisplayMode::getWidth);
	classBoxedDisplayMode->addProperty("height", &BoxedDisplayMode::getHeight);
	classBoxedDisplayMode->addProperty("refreshRate", &BoxedDisplayMode::getRefreshRate);
	classBoxedDisplayMode->addProperty("colorBits", &BoxedDisplayMode::getColorBits);
	registrar->registerClass(classBoxedDisplayMode);

	auto classBoxedSimpleTextureCreateDesc = new AutoRuntimeClass< BoxedSimpleTextureCreateDesc >();
	classBoxedSimpleTextureCreateDesc->addConstructor();
	classBoxedSimpleTextureCreateDesc->addProperty("width", &BoxedSimpleTextureCreateDesc::setWidth, &BoxedSimpleTextureCreateDesc::getWidth);
	classBoxedSimpleTextureCreateDesc->addProperty("height", &BoxedSimpleTextureCreateDesc::setHeight, &BoxedSimpleTextureCreateDesc::getHeight);
	classBoxedSimpleTextureCreateDesc->addProperty("mipCount", &BoxedSimpleTextureCreateDesc::setMipCount, &BoxedSimpleTextureCreateDesc::getMipCount);
	classBoxedSimpleTextureCreateDesc->addProperty("sRGB", &BoxedSimpleTextureCreateDesc::set_sRGB, &BoxedSimpleTextureCreateDesc::get_sRGB);
	classBoxedSimpleTextureCreateDesc->addProperty("immutable", &BoxedSimpleTextureCreateDesc::setImmutable, &BoxedSimpleTextureCreateDesc::getImmutable);
	registrar->registerClass(classBoxedSimpleTextureCreateDesc);

	auto classITexture = new AutoRuntimeClass< ITexture >();
	classITexture->addProperty("mips", &ITexture::getMips);
	registrar->registerClass(classITexture);

	auto classICubeTexture = new AutoRuntimeClass< ICubeTexture >();
	classICubeTexture->addProperty("side", &ICubeTexture::getSide);
	registrar->registerClass(classICubeTexture);

	auto classISimpleTexture = new AutoRuntimeClass< ISimpleTexture >();
	classISimpleTexture->addProperty("width", &ISimpleTexture::getWidth);
	classISimpleTexture->addProperty("height", &ISimpleTexture::getHeight);
	classISimpleTexture->addMethod("lock", &ISimpleTexture_lock);
	classISimpleTexture->addMethod("unlock", &ISimpleTexture::unlock);
	registrar->registerClass(classISimpleTexture);

	auto classIVolumeTexture = new AutoRuntimeClass< IVolumeTexture >();
	classIVolumeTexture->addProperty("width", &IVolumeTexture::getWidth);
	classIVolumeTexture->addProperty("height", &IVolumeTexture::getHeight);
	classIVolumeTexture->addProperty("depth", &IVolumeTexture::getDepth);
	registrar->registerClass(classIVolumeTexture);

	auto classIRenderSystem = new AutoRuntimeClass< IRenderSystem >();
	classIRenderSystem->addStaticMethod("getParameterHandle", &IRenderSystem_getHandle);
	classIRenderSystem->addProperty("displayModeCount", &IRenderSystem::getDisplayModeCount);
	classIRenderSystem->addProperty("currentDisplayMode", &IRenderSystem_getCurrentDisplayMode);
	classIRenderSystem->addProperty("displayAspectRatio", &IRenderSystem::getDisplayAspectRatio);
	classIRenderSystem->addMethod("getDisplayMode", &IRenderSystem_getDisplayMode);
	// classIRenderSystem->addMethod("createVertexBuffer", &IRenderSystem_createVertexBuffer);
	// classIRenderSystem->addMethod("createIndexBuffer", &IRenderSystem_createIndexBuffer);
	// classIRenderSystem->addMethod("createStructBuffer", &IRenderSystem_createStructBuffer);
	classIRenderSystem->addMethod("createSimpleTexture", &IRenderSystem_createSimpleTexture);
	// classIRenderSystem->addMethod("createCubeTexture", &IRenderSystem_createCubeTexture);
	// classIRenderSystem->addMethod("createVolumeTexture", &IRenderSystem_createVolumeTexture);
	registrar->registerClass(classIRenderSystem);

	auto classIRenderView = new AutoRuntimeClass< IRenderView >();
	classIRenderView->addProperty("width", &IRenderView::getWidth);
	classIRenderView->addProperty("height", &IRenderView::getHeight);
	classIRenderView->addProperty("isActive", &IRenderView::isActive);
	classIRenderView->addProperty("isMinimized", &IRenderView::isMinimized);
	classIRenderView->addProperty("isFullScreen", &IRenderView::isFullScreen);
	classIRenderView->addMethod("close", &IRenderView::close);
	classIRenderView->addMethod("showCursor", &IRenderView::showCursor);
	classIRenderView->addMethod("hideCursor", &IRenderView::hideCursor);
	classIRenderView->addMethod("isCursorVisible", &IRenderView::isCursorVisible);
	classIRenderView->addMethod("setGamma", &IRenderView::setGamma);
	registrar->registerClass(classIRenderView);
}

	}
}
