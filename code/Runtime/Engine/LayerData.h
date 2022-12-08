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
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::runtime
{

class IEnvironment;
class Layer;
class Stage;

/*! Abstract stage layer data.
 * \ingroup Runtime
 */
class T_DLLCLASS LayerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const = 0;

	virtual void serialize(ISerializer& s) override;

protected:
	friend class StagePipeline;

	std::wstring m_name;
	bool m_permitTransition = true;
};

}
