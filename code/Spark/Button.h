/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix33.h"
#include "Spark/ColorTransform.h"
#include "Spark/Character.h"

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

/*! Flash button character.
 * \ingroup Spark
 */
class T_DLLCLASS Button : public Character
{
	T_RTTI_CLASS;

public:
	enum StateMasks
	{
		SmHitTest = 1 << 0,
		SmDown = 1 << 1,
		SmOver = 1 << 2,
		SmUp = 1 << 3
	};

	struct ButtonLayer
	{
		uint8_t state = 0;
		uint16_t characterId = 0;
		uint16_t placeDepth = 0;
		Matrix33 placeMatrix = Matrix33::identity();
		ColorTransform cxform;

		void serialize(ISerializer& s);
	};

	typedef AlignedVector< ButtonLayer > button_layers_t;

	/*! Add button layer.
	 *
	 * \param layer Button layer description.
	 */
	void addButtonLayer(const ButtonLayer& layer);

	/*! Get button layers.
	 *
	 * \return Button layers.
	 */
	const button_layers_t& getButtonLayers() const;

	virtual Ref< CharacterInstance > createInstance(
		Context* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	button_layers_t m_layers;
};

	}
}

