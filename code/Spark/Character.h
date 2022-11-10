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
#include "Core/Math/Matrix33.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class CharacterInstance;
class Context;
class Dictionary;

/*! Character definition.
 * \ingroup Spark
 */
class T_DLLCLASS Character : public ISerializable
{
	T_RTTI_CLASS;

public:
	Character();

	/*! Get character unique tag.
	 *
	 * The tag is guaranteed to be unique during the
	 * life-time of the running process even
	 * if multiple players are created
	 * and destroyed during this time.
	 */
	int32_t getCacheTag() const { return m_tag; }

	/*! Create character instance.
	 *
	 * \param context Execution context.
	 * \param dictionary Movie dictionary.
	 * \param parent Parent instance.
	 * \param name Character name.
	 * \param transform Character transform.
	 * \return Character instance.
	 */
	virtual Ref< CharacterInstance > createInstance(
		Context* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform
	) const = 0;

	/*!
	 */
	virtual void serialize(ISerializer& s) override;

private:
	int32_t m_tag;
};

	}
}

