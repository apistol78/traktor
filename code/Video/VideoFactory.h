/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_video_VideoFactory_H
#define traktor_video_VideoFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;

	}

	namespace video
	{

/*! \brief Video resource factory.
 * \ingroup Video
 */
class T_DLLCLASS VideoFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	VideoFactory(render::IRenderSystem* renderSystem);

	virtual const TypeInfoSet getResourceTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const T_OVERRIDE T_FINAL;

	virtual bool isCacheable(const TypeInfo& productType) const T_OVERRIDE T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const T_OVERRIDE T_FINAL;

private:
	Ref< render::IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_video_VideoFactory_H
