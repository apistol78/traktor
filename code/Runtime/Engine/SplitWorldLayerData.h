/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Runtime/Engine/LayerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::scene
{

class Scene;

}

namespace traktor::runtime
{

/*! Split screen world stage layer data.
 * \ingroup Runtime
 */
class T_DLLCLASS SplitWorldLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	SplitWorldLayerData();

	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class StagePipeline;

	resource::Id< scene::Scene > m_scene;
};

}
