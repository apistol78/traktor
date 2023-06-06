/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Render/Frame/RenderGraphTypes.h"
#include "Render/Image2/ImageGraphTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageTargetSet : public Object
{
	T_RTTI_CLASS;

public:
	explicit ImageTargetSet(const std::wstring& name, handle_t persistentHandle, img_handle_t textureIds[RenderGraphTargetSetDesc::MaxColorTargets], const RenderGraphTargetSetDesc& targetSetDesc);

	const std::wstring& getName() const;

	handle_t getPersistentHandle() const;

	img_handle_t getTextureId(int32_t colorIndex) const;

	const RenderGraphTargetSetDesc& getTargetSetDesc() const;

private:
	std::wstring m_name;
	handle_t m_persistentHandle;
	img_handle_t m_textureIds[RenderGraphTargetSetDesc::MaxColorTargets];
	RenderGraphTargetSetDesc m_targetSetDesc;
};

}
