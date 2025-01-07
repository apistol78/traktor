/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Render/Editor/Node.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ShaderGraph;

/*! External shader node.
 * \ingroup Render
 *
 * External shader node is a special kind of node
 * which reference another shader graph, a fragment.
 * When the shader is created the external fragment is
 * loaded and merged into the final shader.
 */
class T_DLLCLASS External : public Node
{
	T_RTTI_CLASS;

public:
	External() = default;

	explicit External(const Guid& fragmentGuid, const ShaderGraph* fragmentGraph);

	virtual ~External();

	void setFragmentGuid(const Guid& fragmentGuid);

	const Guid& getFragmentGuid() const;

	const InputPin* createInputPin(const Guid& id, const std::wstring& name, bool optional);

	const OutputPin* createOutputPin(const Guid& id, const std::wstring& name);

	void removeInputPin(const InputPin* inputPin);

	void removeOutputPin(const OutputPin* outputPin);

	virtual std::wstring getInformation() const override final;

	virtual int getInputPinCount() const override final;

	virtual const InputPin* getInputPin(int index) const override final;

	virtual int getOutputPinCount() const override final;

	virtual const OutputPin* getOutputPin(int index) const override final;

	virtual void serialize(ISerializer& s) override final;

	AlignedVector< InputPin* >& getInputPins() { return m_inputPins; }

	AlignedVector< OutputPin* >& getOutputPins() { return m_outputPins; }

private:
	Guid m_fragmentGuid;
	AlignedVector< InputPin* > m_inputPins;
	AlignedVector< OutputPin* > m_outputPins;
};

}
