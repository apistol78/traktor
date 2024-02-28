/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/Boxes/BoxedAlignedVector.h"
#include "Core/Class/Boxes/BoxedMatrix44.h"
#include "Core/Class/Boxes/BoxedPointer.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Render/Buffer.h"
#include "Render/IBufferView.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ITexture.h"
#include "Render/RenderClassFactory.h"
#include "Render/Context/ProgramParameters.h"
#include "Render/Image2/ImageGraphContext.h"

namespace traktor::render
{
	namespace
	{

class BoxedDataType : public Object
{
	T_RTTI_CLASS;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.DataType", BoxedDataType, Object)

class BoxedDisplayMode : public Object
{
	T_RTTI_CLASS;

public:
	explicit BoxedDisplayMode(const DisplayMode& displayMode)
	:	m_displayMode(displayMode)
	{
	}

	uint32_t getWidth() const { return m_displayMode.width; }

	uint32_t getHeight() const { return m_displayMode.height; }

	uint32_t getDPI() const { return m_displayMode.dpi; }

	float getRefreshRate() const { return m_displayMode.refreshRate; }

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

	explicit BoxedSimpleTextureCreateDesc(const SimpleTextureCreateDesc& stcd)
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

BoxedAllocator< BoxedProgramParameters, 4096 > s_allocBoxedProgramParameters;

Ref< BoxedPointer > Buffer_lock(Buffer* self)
{
	void* ptr = self->lock();
	if (ptr)
		return new BoxedPointer(ptr);
	else
		return nullptr;
}

Ref< BoxedPointer > ITexture_lock(ITexture* self, int32_t side, int32_t level)
{
	ITexture::Lock lock;
	if (self->lock(side, level, lock))
		return new BoxedPointer(lock.bits);
	else
		return nullptr;
}

handle_t IRenderSystem_getHandle(const std::wstring& id)
{
	return getParameterHandle(id);
}

Ref< BoxedDisplayMode > IRenderSystem_getCurrentDisplayMode(IRenderSystem* self, uint32_t display)
{
	return new BoxedDisplayMode(self->getCurrentDisplayMode(display));
}

Ref< BoxedDisplayMode > IRenderSystem_getDisplayMode(IRenderSystem* self, uint32_t display, uint32_t index)
{
	return new BoxedDisplayMode(self->getDisplayMode(display, index));
}

//Ref< StructBuffer > IRenderSystem_createStructBuffer(IRenderSystem* self, const RefArray< BoxedStructElement >& structElements, uint32_t bufferSize, bool dynamic)
//{
//	AlignedVector< StructElement > se;
//
//	se.resize(structElements.size());
//	for (size_t i = 0; i < structElements.size(); ++i)
//		se[i] = structElements[i]->unbox();
//
//	return self->createStructBuffer(se, bufferSize, dynamic);
//}

Ref< ITexture > IRenderSystem_createSimpleTexture(IRenderSystem* self, const BoxedSimpleTextureCreateDesc* stcd)
{
	return self->createSimpleTexture(stcd->unbox(), T_FILE_LINE_W);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderClassFactory", 0, RenderClassFactory, IRuntimeClassFactory)

void RenderClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classBoxedDataType = new AutoRuntimeClass< BoxedDataType >();
	classBoxedDataType->addConstant("DtFloat1", Any::fromInt32(DtFloat1));
	classBoxedDataType->addConstant("DtFloat2", Any::fromInt32(DtFloat2));
	classBoxedDataType->addConstant("DtFloat3", Any::fromInt32(DtFloat3));
	classBoxedDataType->addConstant("DtFloat4", Any::fromInt32(DtFloat4));
	classBoxedDataType->addConstant("DtByte4", Any::fromInt32(DtByte4));
	classBoxedDataType->addConstant("DtByte4N", Any::fromInt32(DtByte4N));
	classBoxedDataType->addConstant("DtShort2", Any::fromInt32(DtShort2));
	classBoxedDataType->addConstant("DtShort4", Any::fromInt32(DtShort4));
	classBoxedDataType->addConstant("DtShort2N", Any::fromInt32(DtShort2N));
	classBoxedDataType->addConstant("DtShort4N", Any::fromInt32(DtShort4N));
	classBoxedDataType->addConstant("DtHalf2", Any::fromInt32(DtHalf2));
	classBoxedDataType->addConstant("DtHalf4", Any::fromInt32(DtHalf4));
	classBoxedDataType->addConstant("DtInteger1", Any::fromInt32(DtInteger1));
	classBoxedDataType->addConstant("DtInteger2", Any::fromInt32(DtInteger2));
	classBoxedDataType->addConstant("DtInteger3", Any::fromInt32(DtInteger3));
	classBoxedDataType->addConstant("DtInteger4", Any::fromInt32(DtInteger4));
	registrar->registerClass(classBoxedDataType);

	auto classBoxedDisplayMode = new AutoRuntimeClass< BoxedDisplayMode >();
	classBoxedDisplayMode->addProperty("width", &BoxedDisplayMode::getWidth);
	classBoxedDisplayMode->addProperty("height", &BoxedDisplayMode::getHeight);
	classBoxedDisplayMode->addProperty("dpi", &BoxedDisplayMode::getDPI);
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

	auto classBoxedProgramParameters = new AutoRuntimeClass< BoxedProgramParameters >();
	classBoxedProgramParameters->addMethod("setFloatParameter", &BoxedProgramParameters::setFloatParameter);
	classBoxedProgramParameters->addMethod("setVectorParameter", &BoxedProgramParameters::setVectorParameter);
	classBoxedProgramParameters->addMethod("setVectorArrayParameter", &BoxedProgramParameters::setVectorArrayParameter);
	classBoxedProgramParameters->addMethod("setMatrixParameter", &BoxedProgramParameters::setMatrixParameter);
	classBoxedProgramParameters->addMethod("setMatrixArrayParameter", &BoxedProgramParameters::setMatrixArrayParameter);
	classBoxedProgramParameters->addMethod("setTextureParameter", &BoxedProgramParameters::setTextureParameter);
	classBoxedProgramParameters->addMethod("setBufferViewParameter", &BoxedProgramParameters::setBufferViewParameter);
	classBoxedProgramParameters->addMethod("setStencilReference", &BoxedProgramParameters::setStencilReference);
	registrar->registerClass(classBoxedProgramParameters);

	auto classBuffer = new AutoRuntimeClass< Buffer >();
	classBuffer->addProperty("bufferSize", &Buffer::getBufferSize);
	classBuffer->addMethod("lock", &Buffer_lock);
	classBuffer->addMethod("unlock", &Buffer::unlock);
	registrar->registerClass(classBuffer);

	auto classITexture = new AutoRuntimeClass< ITexture >();
	classITexture->addMethod("lock", &ITexture_lock);
	classITexture->addMethod("unlock", &ITexture::unlock);
	registrar->registerClass(classITexture);

	auto classIRenderSystem = new AutoRuntimeClass< IRenderSystem >();
	classIRenderSystem->addStaticMethod("getParameterHandle", &IRenderSystem_getHandle);
	classIRenderSystem->addProperty("displayCount", &IRenderSystem::getDisplayCount);
	classIRenderSystem->addMethod("getDisplayModeCount", &IRenderSystem::getDisplayModeCount);
	classIRenderSystem->addMethod("getDisplayMode", &IRenderSystem_getDisplayMode);
	classIRenderSystem->addMethod("getCurrentDisplayMode", &IRenderSystem_getCurrentDisplayMode);
	classIRenderSystem->addMethod("getDisplayAspectRatio", &IRenderSystem::getDisplayAspectRatio);
	classIRenderSystem->addMethod("createSimpleTexture", &IRenderSystem_createSimpleTexture);
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

	auto classImageGraphContext = new AutoRuntimeClass< ImageGraphContext >();
	//classImageGraphContext->addMethod("setFloatParameter", &ImageGraphContext::setFloatParameter);
	//classImageGraphContext->addMethod("setVectorParameter", &ImageGraphContext::setVectorParameter);
	registrar->registerClass(classImageGraphContext);
}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramParameters", BoxedProgramParameters, Object)

BoxedProgramParameters::BoxedProgramParameters(ProgramParameters* programParameters)
:	m_programParameters(programParameters)
{
}

void BoxedProgramParameters::setProgramParameters(ProgramParameters* programParameters)
{
	m_programParameters = programParameters;
}

void BoxedProgramParameters::setFloatParameter(const handle_t handle, float param)
{
	m_programParameters->setFloatParameter(handle, param);
}

void BoxedProgramParameters::setVectorParameter(const handle_t handle, const Vector4& param)
{
	m_programParameters->setVectorParameter(handle, param);
}

void BoxedProgramParameters::setVectorArrayParameter(const handle_t handle, const AlignedVector< Vector4 >& param)
{
	m_programParameters->setVectorArrayParameter(handle, param.c_ptr(), (int)param.size());
}

void BoxedProgramParameters::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	m_programParameters->setMatrixParameter(handle, param);
}

void BoxedProgramParameters::setMatrixArrayParameter(handle_t handle, const AlignedVector< Matrix44 >& param)
{
	m_programParameters->setMatrixArrayParameter(handle, param.c_ptr(), (int)param.size());
}

void BoxedProgramParameters::setTextureParameter(const handle_t handle, ITexture* texture)
{
	m_programParameters->setTextureParameter(handle, texture);
}

void BoxedProgramParameters::setBufferViewParameter(const handle_t handle, const IBufferView* bufferView)
{
	m_programParameters->setBufferViewParameter(handle, bufferView);
}

void BoxedProgramParameters::setStencilReference(uint32_t stencilReference)
{
	m_programParameters->setStencilReference(stencilReference);
}

void* BoxedProgramParameters::operator new (size_t size)
{
	return s_allocBoxedProgramParameters.alloc();
}

void BoxedProgramParameters::operator delete (void* ptr)
{
	s_allocBoxedProgramParameters.free(ptr);
}

}
