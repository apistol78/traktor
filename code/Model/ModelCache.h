/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Date/DateTime.h"
#include "Core/Io/Path.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::model
{

class Model;

/*!
 * \ingroup Model
 */
class T_DLLCLASS ModelCache : public ISingleton
{
public:
	static ModelCache& getInstance();

	virtual void destroy();

	Ref< const Model > get(const Path& cachePath, const Path& fileName, const std::wstring& filter);

	Ref< Model > getMutable(const Path& cachePath, const Path& fileName, const std::wstring& filter);

private:
	struct ModelWithStamp
	{
		Ref< const Model > model;
		DateTime timeStamp;
	};

	Semaphore m_lock;
	SmallMap< std::pair< Path, std::wstring >, ModelWithStamp > m_models;
};

}
