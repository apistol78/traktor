/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Sound/Processor/InputPin.h"
#include "Sound/Processor/Node.h"
#include "Sound/Processor/OutputPin.h"
#include "Sound/Processor/ProcessorTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*! Immutable processor graph node.
 * \ingroup Sound
 */
class T_DLLCLASS ImmutableNode : public Node
{
	T_RTTI_CLASS;

public:
	struct InputPinDesc
	{
		const wchar_t* name;
		NodePinType type;
		bool optional;
	};

	struct OutputPinDesc
	{
		const wchar_t* name;
		NodePinType type;
	};

	explicit ImmutableNode(const InputPinDesc* inputPins, const OutputPinDesc* outputPins);

	virtual size_t getInputPinCount() const override final;

	virtual const InputPin* getInputPin(size_t index) const override final;

	virtual size_t getOutputPinCount() const override final;

	virtual const OutputPin* getOutputPin(size_t index) const override final;

private:
	RefArray< InputPin > m_inputPins;
	RefArray< OutputPin > m_outputPins;

	ImmutableNode& operator = (const ImmutableNode&) { T_FATAL_ERROR; return *this; }
};

}
