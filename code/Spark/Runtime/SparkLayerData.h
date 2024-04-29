/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Runtime/Engine/LayerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spark
{

class Movie;

/*! Stage Spark layer persistent data.
 * \ingroup Spark
 */
class T_DLLCLASS SparkLayerData : public runtime::LayerData
{
	T_RTTI_CLASS;

public:
	SparkLayerData();

	virtual Ref< runtime::Layer > createInstance(runtime::Stage* stage, runtime::IEnvironment* environment) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class SparkLayerPipeline;

	resource::Id< Movie > m_movie;
	SmallMap< std::wstring, resource::Id< Movie > > m_externalMovies;
	bool m_clearBackground = false;
	bool m_enableSound = false;
};

}
