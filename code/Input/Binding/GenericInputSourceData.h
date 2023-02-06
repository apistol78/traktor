/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Input/InputTypes.h"
#include "Input/Binding/IInputSourceData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

/*! Generic input source.
 * \ingroup Input
 */
class T_DLLCLASS GenericInputSourceData : public IInputSourceData
{
	T_RTTI_CLASS;

public:
	GenericInputSourceData();

	explicit GenericInputSourceData(
		InputCategory category,
		InputDefaultControlType controlType,
		bool analogue,
		bool normalize
	);

	explicit GenericInputSourceData(
		InputCategory category,
		int32_t index,
		InputDefaultControlType controlType,
		bool analogue,
		bool normalize
	);

	void setCategory(InputCategory category);

	InputCategory getCategory() const;

	void setControlType(InputDefaultControlType controlType);

	InputDefaultControlType getControlType() const;

	void setAnalogue(bool analogue);

	bool isAnalogue() const;

	void setNormalize(bool normalize);

	bool normalize() const;

	void setIndex(int32_t index);

	int32_t getIndex() const;

	virtual Ref< IInputSource > createInstance(DeviceControlManager* deviceControlManager) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	InputCategory m_category;
	InputDefaultControlType m_controlType;
	bool m_analogue;
	bool m_normalize;
	int32_t m_index;
};

}
