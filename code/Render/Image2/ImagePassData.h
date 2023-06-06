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
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/Types.h"

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

class ImagePass;
class ImagePassStepData;
class IRenderSystem;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImagePassData : public ISerializable
{
    T_RTTI_CLASS;

public:
    Ref< const ImagePass > createInstance(resource::IResourceManager* resourceManager, IRenderSystem* renderSystem) const;

    virtual void serialize(ISerializer& s) override final;

private:
    friend class ImageGraphPipeline;

    std::wstring m_name;
    int32_t m_outputTargetSet = -1;
    int32_t m_outputSBuffer = -1;
    Clear m_clear;
    RefArray< ImagePassStepData > m_steps;
};

}
