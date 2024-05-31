/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::spark
{

class AccShapeResources : public Object
{
	T_RTTI_CLASS;

public:
	bool create(resource::IResourceManager* resourceManager);

	void destroy();

private:
	friend class AccDisplayRenderer;
	friend class AccQuad;
	friend class AccShape;

	resource::Proxy< render::Shader > m_shaderSolid;
	resource::Proxy< render::Shader > m_shaderTextured;
	resource::Proxy< render::Shader > m_shaderLine;
	resource::Proxy< render::Shader > m_shaderIncrementMask;
	resource::Proxy< render::Shader > m_shaderDecrementMask;
};

}
