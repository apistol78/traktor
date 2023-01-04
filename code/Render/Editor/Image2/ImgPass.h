/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "Render/Editor/Node.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IImgStep;

/*! Image pass definition
 * \ingroup Render
 *
 * A pass contain multiple steps which describe how this pass
 * should be rendered.
 */
class T_DLLCLASS ImgPass : public Node
{
	T_RTTI_CLASS;

public:
	ImgPass();

	virtual ~ImgPass();

	const std::wstring& getName() const;

	const Clear& getClear() const;

	const RefArray< IImgStep >& getSteps() const;

	virtual int getInputPinCount() const override final;

	virtual const InputPin* getInputPin(int index) const override final;

	virtual int getOutputPinCount() const override final;

	virtual const OutputPin* getOutputPin(int index) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	AlignedVector< InputPin* > m_inputPins;
	AlignedVector< OutputPin* > m_outputPins;
	Clear m_clear;
	RefArray< IImgStep > m_steps;

	void refresh();
};

}
