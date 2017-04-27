/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SequenceTextureResource_H
#define traktor_render_SequenceTextureResource_H

#include <list>
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

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

class ITexture;

/*! \brief Sequence texture resource.
 * \ingroup Render
 */
class T_DLLCLASS SequenceTextureResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	SequenceTextureResource();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class SequenceTexturePipeline;
	friend class SequenceTextureFactory;

	float m_rate;
	std::list< resource::Id< ITexture > > m_textures;
};

	}
}

#endif	// traktor_render_SequenceTextureResource_H
