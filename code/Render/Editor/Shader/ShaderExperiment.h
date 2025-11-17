/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Guid.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS ShaderExperiment : public ISerializable
{
    T_RTTI_CLASS;

public:
    struct Data
    {
        std::wstring name;
        Guid structDeclaration;
        int32_t count = 1;

        void serialize(ISerializer& s);
    };

    struct Pass
    {
        std::wstring technique;
        int32_t workSize = 1;

        void serialize(ISerializer& s);
    };

    const Guid& getShader() const { return m_shader; }

    const AlignedVector< Data >& getData() const { return m_data; }

    const AlignedVector< Pass >& getPasses() const { return m_passes; }

    virtual void serialize(ISerializer& s) override final;

private:
    Guid m_shader;
    AlignedVector< Data > m_data;
    AlignedVector< Pass > m_passes;
};

}
