/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Render/IVertexLayout.h"

namespace traktor::render
{

/*!
 * \ingroup Vrfy
 */
class VertexLayoutVrfy : public IVertexLayout
{
	T_RTTI_CLASS;

public:
	const VertexLayoutVrfy(const IVertexLayout* wrappedVertexLayout, uint32_t vertexSize);

	const IVertexLayout* getWrappedVertexLayout() const { return m_wrappedVertexLayout; }

	uint32_t getVertexSize() const { return m_vertexSize; }

private:
	Ref< const IVertexLayout > m_wrappedVertexLayout;
	uint32_t m_vertexSize = 0;
};

}
