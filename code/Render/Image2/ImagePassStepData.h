/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class ImagePassStep;
class IRenderSystem;
class Shader;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImagePassStepData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< const ImagePassStep > createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const = 0;

	virtual void serialize(ISerializer& s) override;

protected:
	friend class ImageGraphPipeline;

	struct Source
	{
		std::wstring id;
		std::wstring parameter;

		void serialize(ISerializer& s);
	};

	resource::Id< Shader > m_shader;
	AlignedVector< Source > m_textureSources;
	AlignedVector< Source > m_structBufferSources;
};

}
