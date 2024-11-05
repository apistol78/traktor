/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/StaticVector.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/Shader/TypedOutputPin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Script shader node.
 * \ingroup Render
 *
 * Script shader nodes enabled custom
 * function nodes where functionality
 * is written in pure shader code such as HLSL, GLSL
 * etc.
 * 
 * A script node with specified technique name
 * is assumed to be a Compute root node.
 */
class T_DLLCLASS Script : public Node
{
	T_RTTI_CLASS;

public:
	enum Domain : int32_t
	{
		Undefined = 0,
		Vertex = 1,
		Pixel = 2,
		Compute = 3,
		Callable = 4
	};

	Script();

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setTechnique(const std::wstring& technique);

	const std::wstring& getTechnique() const;

	void setDomain(Domain domain);

	Domain getDomain() const;

	const int32_t* getLocalSize() const;

	bool useRayTracing() const;

	const AlignedVector< Guid >& getIncludes() const;

	void setScript(const std::wstring& script);

	const std::wstring& getScript() const;

	const InputPin* addInputPin(const Guid& id, const std::wstring& name);

	const OutputPin* addOutputPin(const Guid& id, const std::wstring& name, ParameterType type);

	void removeInputPin(const std::wstring& name);

	void removeOutputPin(const std::wstring& name);

	void removeAllInputPins();

	void removeAllOutputPins();

	ParameterType getOutputPinType(int index) const;

	virtual std::wstring getInformation() const override final;

	virtual int getInputPinCount() const override final;

	virtual const InputPin* getInputPin(int index) const override final;

	virtual int getOutputPinCount() const override final;

	virtual const OutputPin* getOutputPin(int index) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	std::wstring m_technique;
	Domain m_domain = Undefined;
	int32_t m_localSize[3];
	bool m_useRayTracing = false;
	AlignedVector< Guid > m_includes;
	StaticVector< InputPin, 64 > m_inputPins;
	StaticVector< TypedOutputPin, 64 > m_outputPins;
	std::wstring m_script;
};

}
