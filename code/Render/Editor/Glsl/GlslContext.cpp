/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Glsl/GlslContext.h"

#include "Core/Log/Log.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/String.h"
#include "Render/Editor/Glsl/GlslImage.h"
#include "Render/Editor/Glsl/GlslSampler.h"
#include "Render/Editor/Glsl/GlslShader.h"
#include "Render/Editor/Glsl/GlslStorageBuffer.h"
#include "Render/Editor/Glsl/GlslTexture.h"
#include "Render/Editor/Glsl/GlslUniformBuffer.h"
#include "Render/Editor/GraphTraverse.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor::render
{
namespace
{

const SamplerState c_samplerPointState = {
	.minFilter = Filter::Point,
	.mipFilter = Filter::Point,
	.magFilter = Filter::Point,
	.addressU = Address::Wrap,
	.addressV = Address::Wrap,
	.addressW = Address::Wrap,
	.compare = CompareFunction::None,
	.mipBias = 0.0f,
	.ignoreMips = false,
	.useAnisotropic = false
};

const SamplerState c_samplerLinearState = {
	.minFilter = Filter::Linear,
	.mipFilter = Filter::Linear,
	.magFilter = Filter::Linear,
	.addressU = Address::Wrap,
	.addressV = Address::Wrap,
	.addressW = Address::Wrap,
	.compare = CompareFunction::None,
	.mipBias = 0.0f,
	.ignoreMips = false,
	.useAnisotropic = false
};

std::wstring getClassNameOnly(const Object* o)
{
	const std::wstring qn = type_name(o);
	const size_t p = qn.find_last_of('.');
	return qn.substr(p + 1);
}

}

GlslContext::GlslContext(const ShaderGraph* shaderGraph, const PropertyGroup* settings, const IProgramCompiler::IModuleAccess& moduleAccess)
	: m_shaderGraph(shaderGraph)
	, m_settings(settings)
	, m_moduleAccess(moduleAccess)
	, m_vertexShader(GlslShader::StVertex)
	, m_fragmentShader(GlslShader::StFragment)
	, m_computeShader(GlslShader::StCompute)
	, m_currentShader(nullptr)
{
	m_layout.addStatic(new GlslTexture(L"__bindlessTextures2D__", GlslResource::Set::BindlessTextures, GlslResource::BsAll, GlslType::Texture2D, true), /* binding */ 0);
	m_layout.addStatic(new GlslTexture(L"__bindlessTextures3D__", GlslResource::Set::BindlessTextures, GlslResource::BsAll, GlslType::Texture3D, true), /* binding */ 0);
	m_layout.addStatic(new GlslTexture(L"__bindlessTexturesCube__", GlslResource::Set::BindlessTextures, GlslResource::BsAll, GlslType::TextureCube, true), /* binding */ 0);
	m_layout.addStatic(new GlslImage(L"__bindlessImages2D__", GlslResource::Set::BindlessImages, GlslResource::BsAll, GlslType::Image2D, true), /* binding */ 1);
	m_layout.addStatic(new GlslImage(L"__bindlessImages3D__", GlslResource::Set::BindlessImages, GlslResource::BsAll, GlslType::Image3D, true), /* binding */ 1);
	m_layout.addStatic(new GlslImage(L"__bindlessImagesCube__", GlslResource::Set::BindlessImages, GlslResource::BsAll, GlslType::ImageCube, true), /* binding */ 1);
	// #note Binding 2 is bindless buffers, which is only accessible from shader modules.
	m_layout.addStatic(new GlslUniformBuffer(L"UbOnce", GlslResource::Set::Default, 0), /* binding */ 3);
	m_layout.addStatic(new GlslUniformBuffer(L"UbFrame", GlslResource::Set::Default, 0), /* binding */ 4);
	m_layout.addStatic(new GlslUniformBuffer(L"UbDraw", GlslResource::Set::Default, 0), /* binding */ 5);
}

Node* GlslContext::getInputNode(const InputPin* inputPin)
{
	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	return sourcePin ? sourcePin->getNode() : nullptr;
}

Node* GlslContext::getInputNode(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	return inputPin ? getInputNode(inputPin) : nullptr;
}

bool GlslContext::emit(Node* node)
{
	// In case we're in failure state we ignore recursing further.
	if (!m_error.empty())
		return false;

	bool allOutputsEmitted = true;

	// Check if all active outputs of node already has been emitted.
	const int32_t outputPinCount = node->getOutputPinCount();
	for (int32_t i = 0; i < outputPinCount; ++i)
	{
		const OutputPin* outputPin = node->getOutputPin(i);
		T_ASSERT(outputPin != nullptr);

		if (m_shaderGraph->getDestinationCount(outputPin) == 0)
			continue;

		GlslVariable* variable = m_currentShader->getVariable(node->getOutputPin(i));
		if (!variable)
		{
			allOutputsEmitted = false;
			break;
		}
	}
	if (outputPinCount > 0 && allOutputsEmitted)
		return true;

	const bool result = m_emitter.emit(*this, node);
	if (!result)
	{
		// Only log first failure point; all recursions will also fail.
		if (m_error.empty())
			m_error = getCurrentScope();
	}

	return result;
}

GlslVariable* GlslContext::emitInput(const InputPin* inputPin)
{
	// In case we're in failure state we ignore recursing further.
	if (!m_error.empty())
		return nullptr;

	const OutputPin* sourcePin = m_shaderGraph->findSourcePin(inputPin);
	if (!sourcePin)
		return nullptr;

	// Check if node's output already has been emitted.
	GlslVariable* variable = m_currentShader->getVariable(sourcePin);
	if (variable)
		return variable;

	Node* node = sourcePin->getNode();

	m_emitScope.push_back(Scope(
		inputPin,
		sourcePin));

	const bool result = m_emitter.emit(*this, node);
	if (result)
	{
		variable = m_currentShader->getVariable(sourcePin);
		T_ASSERT(variable);
	}
	else
	{
		// Only log first failure point; all recursions will also fail.
		if (m_error.empty())
		{
			// Format chain to properly indicate source of error.
			StringOutputStream ss;
			for (std::list< Scope >::const_reverse_iterator i = m_emitScope.rbegin(); i != m_emitScope.rend(); ++i)
				ss << getClassNameOnly(i->outputPin->getNode()) << L"[" << i->outputPin->getName() << L"] <-- [" << i->inputPin->getName() << L"]";
			ss << getClassNameOnly(m_emitScope.front().inputPin->getNode());
			m_error = ss.str();
		}
	}

	m_emitScope.pop_back();
	return variable;
}

GlslVariable* GlslContext::emitInput(Node* node, const std::wstring& inputPinName)
{
	const InputPin* inputPin = node->findInputPin(inputPinName);
	T_ASSERT(inputPin);
	return emitInput(inputPin);
}

GlslVariable* GlslContext::emitOutput(const OutputPin* outputPin, GlslType type)
{
	GlslVariable* out = m_currentShader->createTemporaryVariable(outputPin, type);
	T_ASSERT(out);
	return out;
}

GlslVariable* GlslContext::emitOutput(Node* node, const std::wstring& outputPinName, GlslType type)
{
	const OutputPin* outputPin = node->findOutputPin(outputPinName);
	T_ASSERT(outputPin);
	return emitOutput(outputPin, type);
}

void GlslContext::findNonDependentOutputs(Node* node, const std::wstring& inputPinName, const AlignedVector< const OutputPin* >& dependentOutputPins, AlignedVector< const OutputPin* >& outOutputPins) const
{
	getNonDependentOutputs(m_shaderGraph, node->findInputPin(inputPinName), dependentOutputPins, outOutputPins);
}

void GlslContext::findCommonOutputs(Node* node, const std::wstring& inputPin1, const std::wstring& inputPin2, AlignedVector< const OutputPin* >& outOutputPins) const
{
	AlignedVector< const InputPin* > inputPins(2);
	inputPins[0] = node->findInputPin(inputPin1);
	inputPins[1] = node->findInputPin(inputPin2);
	getMergingOutputs(m_shaderGraph, inputPins, outOutputPins);
}

void GlslContext::findCommonOutputs(const AlignedVector< const InputPin* >& inputPins, AlignedVector< const OutputPin* >& outOutputPins) const
{
	getMergingOutputs(m_shaderGraph, inputPins, outOutputPins);
}

bool GlslContext::isConnected(const OutputPin* outputPin) const
{
	return m_shaderGraph->getDestinationCount(outputPin) > 0;
}

void GlslContext::enterVertex()
{
	m_currentShader = &m_vertexShader;
}

bool GlslContext::inVertex() const
{
	return bool(m_currentShader == &m_vertexShader);
}

void GlslContext::enterFragment()
{
	m_currentShader = &m_fragmentShader;
}

bool GlslContext::inFragment() const
{
	return bool(m_currentShader == &m_fragmentShader);
}

void GlslContext::enterCompute()
{
	m_currentShader = &m_computeShader;
}

bool GlslContext::inCompute() const
{
	return bool(m_currentShader == &m_computeShader);
}

uint32_t GlslContext::getBindStage() const
{
	if (inVertex())
		return GlslResource::BsVertex;
	else if (inFragment())
		return GlslResource::BsFragment;
	else if (inCompute())
		return GlslResource::BsCompute;
	else
		return GlslResource::BsVertex | GlslResource::BsFragment | GlslResource::BsCompute;
}

const PropertyGroup* GlslContext::getSettings() const
{
	return m_settings;
}

GlslRequirements& GlslContext::requirements()
{
	return m_requirements;
}

const GlslRequirements& GlslContext::requirements() const
{
	return m_requirements;
}

bool GlslContext::allocateInterpolator(int32_t width, int32_t& outId, int32_t& outOffset)
{
	const int32_t lastId = int32_t(m_interpolatorMap.size());

	for (int32_t i = 0; i < lastId; ++i)
	{
		uint8_t& occupied = m_interpolatorMap[i];
		if (width <= 4 - occupied)
		{
			outId = i;
			outOffset = occupied;
			occupied += width;
			return false;
		}
	}

	outId = lastId;
	outOffset = 0;

	m_interpolatorMap.push_back(width);
	return true;
}

bool GlslContext::addParameter(const std::wstring& name, ParameterType type, int32_t length, UpdateFrequency frequency)
{
	if (haveParameter(name))
		return false;
	m_parameters.push_back({ name,
		type,
		length,
		frequency });
	return true;
}

bool GlslContext::haveParameter(const std::wstring& name) const
{
	for (const auto& parameter : m_parameters)
		if (parameter.name == name)
			return true;
	return false;
}

GlslLayout& GlslContext::getLayout()
{
	return m_layout;
}

void GlslContext::setRenderState(const RenderState& renderState)
{
	m_renderState = renderState;
}

void GlslContext::registerModule(const Guid& moduleId)
{
	const std::wstring text = m_moduleAccess.getText(moduleId);
	getShader().addModule(moduleId, text);

	for (const auto it : m_moduleAccess.getSamplers(moduleId))
		if (getLayout().getByName(it.first) == nullptr)
			getLayout().add(
				new GlslSampler(
					it.first,
					GlslResource::Set::Default,
					getBindStage(),
					it.second));
		else
			log::warning << L"Sampler defined in module \"" << it.first << L"\" already exist in layout." << Endl;
}

void GlslContext::pushError(const std::wstring& errorMessage)
{
	m_errorMessages.push_back({ errorMessage, getCurrentScope() });
}

std::wstring GlslContext::getErrorReport() const
{
	StringOutputStream ss;
	for (auto errorMessage : m_errorMessages)
		ss << errorMessage.message << L" (" << errorMessage.scope << L")" << Endl;
	return ss.str();
}

std::wstring GlslContext::getCurrentScope() const
{
	StringOutputStream ss;
	if (!m_emitScope.empty())
	{
		for (std::list< Scope >::const_reverse_iterator i = m_emitScope.rbegin(); i != m_emitScope.rend(); ++i)
			ss << getClassNameOnly(i->outputPin->getNode()) << L"[" << i->outputPin->getName() << L"] <-- [" << i->inputPin->getName() << L"]";
		ss << getClassNameOnly(m_emitScope.front().inputPin->getNode());
	}
	return ss.str();
}

}
