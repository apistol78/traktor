/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <stack>
#include "Core/Log/Log.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/GraphTraverse.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/INodeTraits.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphEvaluator.h"
#include "Render/Editor/Shader/ShaderGraphValidator.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct CollectVisitor
{
	std::set< const Node* > m_nodes;

	bool operator () (Node* node)
	{
		m_nodes.insert(node);
		return true;
	}

	bool operator () (Edge* edge) {
		return true;
	}
};

class Report
{
public:
	Report(std::vector< const Node* >* outErrorNodes)
	:	m_errorCount(0)
	,	m_outErrorNodes(outErrorNodes)
	{
	}

	void addError(const std::wstring& errorStr, const Node* errorNode = 0)
	{
		log::error << L"(" << ++m_errorCount << L") : " << errorStr << Endl;
		if (m_outErrorNodes && errorNode)
			m_outErrorNodes->push_back(errorNode);
	}

	int getErrorCount() const
	{
		return m_errorCount;
	}

private:
	int32_t m_errorCount;
	std::vector< const Node* >* m_outErrorNodes;
};

class Specification
{
public:
	virtual void check(Report& outReport, const ShaderGraph* shaderGraph, const std::set< const Node* >& activeNodes) = 0;
};

class EdgeNodes : public Specification
{
public:
	virtual void check(Report& outReport, const ShaderGraph* shaderGraph, const std::set< const Node* >& activeNodes)
	{
		const RefArray< Node >& nodes = shaderGraph->getNodes();

		const RefArray< Edge >& edges = shaderGraph->getEdges();
		for (RefArray< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
		{
			if (!(*i)->getSource())
			{
				outReport.addError(L"Edge referencing invalid node (no source)");
				continue;
			}
			if (!(*i)->getSource()->getNode())
			{
				outReport.addError(L"Edge referencing invalid node (no source node)");
				continue;
			}
			if (!(*i)->getDestination())
			{
				outReport.addError(L"Edge referencing invalid node (no destination)");
				continue;
			}
			if (!(*i)->getDestination()->getNode())
			{
				outReport.addError(L"Edge referencing invalid node (no destination node)");
				continue;
			}
			if (std::find(nodes.begin(), nodes.end(), (*i)->getSource()->getNode()) == nodes.end())
				outReport.addError(L"Edge referencing invalid node (source, " + std::wstring(type_name((*i)->getSource()->getNode())) + L")");
			if (std::find(nodes.begin(), nodes.end(), (*i)->getDestination()->getNode()) == nodes.end())
				outReport.addError(L"Edge referencing invalid node (destination, " + std::wstring(type_name((*i)->getDestination()->getNode())) + L")");
		}
	}
};

class UniqueTechniques : public Specification
{
public:
	virtual void check(Report& outReport, const ShaderGraph* shaderGraph, const std::set< const Node* >& activeNodes)
	{
		std::map< std::wstring, uint32_t > techniqueCount;
		for (auto node : shaderGraph->getNodes())
		{
			if (PixelOutput* pixelOutput = dynamic_type_cast< PixelOutput* >(node))
			{
				// If pixel output node's "Enable" pin connected then we cannot ensure uniqueness.
				if (shaderGraph->findSourcePin(pixelOutput->findInputPin(L"Enable")) != nullptr)
					continue;
				if (++techniqueCount[pixelOutput->getTechnique()] > 1)
					outReport.addError(L"Technique names clashing", pixelOutput);
			}
		}
	}
};

class NonOptionalInputs : public Specification
{
public:
	virtual void check(Report& outReport, const ShaderGraph* shaderGraph, const std::set< const Node* >& activeNodes)
	{
		for (auto activeNode : activeNodes)
		{
			const int inputPinCount = activeNode->getInputPinCount();
			for (int j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = activeNode->getInputPin(j);
				if (!inputPin->isOptional() && !shaderGraph->findSourcePin(inputPin))
					outReport.addError(L"Input pin \"" + inputPin->getName() + L"\" of " + activeNode->getId().format() + L" (" + type_name(activeNode) + L") not connected.", activeNode);
			}
		}
	}
};

class SwizzlePatterns : public Specification
{
public:
	virtual void check(Report& outReport, const ShaderGraph* shaderGraph, const std::set< const Node* >& activeNodes)
	{
		for (std::set< const Node* >::const_iterator i = activeNodes.begin(); i != activeNodes.end(); ++i)
		{
			if (is_a< Swizzle >(*i))
			{
				std::wstring swizzle = static_cast< const Swizzle* >(*i)->get();
				if (swizzle.empty() || swizzle.length() > 4)
					outReport.addError(L"Invalid swizzle pattern, invalid length", *i);
				else
				{
					for (std::wstring::const_iterator j = swizzle.begin(); j != swizzle.end(); ++j)
					{
						if (std::wstring(L"xyzw01").find(std::tolower(*j)) == std::wstring::npos)
						{
							outReport.addError(L"Invalid swizzle pattern, unknown channel", *i);
							break;
						}
					}
				}
			}
		}
	}
};

class ParameterNames : public Specification
{
public:
	virtual void check(Report& outReport, const ShaderGraph* shaderGraph, const std::set< const Node* >& activeNodes)
	{
		for (std::set< const Node* >::const_iterator i = activeNodes.begin(); i != activeNodes.end(); ++i)
		{
			std::wstring parameterName;

			if (is_a< Uniform >(*i))
				parameterName = static_cast< const Uniform* >(*i)->getParameterName();
			else if (is_a< IndexedUniform >(*i))
				parameterName = static_cast< const IndexedUniform* >(*i)->getParameterName();
			else if (is_a< Branch >(*i))
				parameterName = static_cast< const Branch* >(*i)->getParameterName();
			else
				continue;

			if (parameterName.empty())
				outReport.addError(L"Invalid node name, no parameter name", *i);
		}
	}
};

class PortNames : public Specification
{
public:
	virtual void check(Report& outReport, const ShaderGraph* shaderGraph, const std::set< const Node* >& activeNodes)
	{
		std::set< std::wstring > usedInputNames, usedOutputNames;
		for (auto node : activeNodes)
		{
			std::set< std::wstring >* usedNames;
			std::wstring portName;

			if (is_a< InputPort >(node))
			{
				usedNames = &usedInputNames;
				portName = static_cast< const InputPort* >(node)->getName();
			}
			else if (is_a< OutputPort >(node))
			{
				usedNames = &usedOutputNames;
				portName = static_cast< const OutputPort* >(node)->getName();
			}
			else
				continue;

			if (portName.empty())
				outReport.addError(L"Invalid port name, no name", node);
			else
			{
				if (usedNames->find(portName) != usedNames->end())
					outReport.addError(L"Port name \"" + portName + L"\" already in use", node);
				else
					usedNames->insert(portName);
			}
		}
	}
};

class NoPorts : public Specification
{
public:
	virtual void check(Report& outReport, const ShaderGraph* ShaderGraph, const std::set< const Node* >& activeNodes)
	{
		for (std::set< const Node* >::const_iterator i = activeNodes.begin(); i != activeNodes.end(); ++i)
		{
			if (is_a< InputPort >(*i) || is_a< OutputPort >(*i))
				outReport.addError(L"Cannot have Input- or OutputPort in non-fragment shader", *i);
		}
	}
};

class NoMetaNodes : public Specification
{
public:
	virtual void check(Report& outReport, const ShaderGraph* ShaderGraph, const std::set< const Node* >& activeNodes)
	{
		for (std::set< const Node* >::const_iterator i = activeNodes.begin(); i != activeNodes.end(); ++i)
		{
			if (is_a< Branch >(*i))
				outReport.addError(L"Cannot have Branch node in program shader", *i);
		}
	}
};

class VariableNames : public Specification
{
public:
	virtual void check(Report& outReport, const ShaderGraph* shaderGraph, const std::set< const Node* >& activeNodes)
	{
		// Ensure all variables has a name.
		for (auto activeNode : activeNodes)
		{
			if (const Variable * variableNode = dynamic_type_cast<const Variable*>(activeNode))
			{
				if (variableNode->getName().empty())
					outReport.addError(L"Invalid variable name.", variableNode);
			}
		}

		// Ensure variables are written only once.
		std::set< std::wstring > written;
		for (auto activeNode : activeNodes)
		{
			if (const Variable * variableNode = dynamic_type_cast<const Variable*>(activeNode))
			{
				if (shaderGraph->findSourcePin(variableNode->findInputPin(L"Input")) != nullptr)
				{
					const auto& name = variableNode->getName();
					if (written.find(name) != written.end())
						outReport.addError(L"Variable \"" + name + L"\" already being written to.");
					else
						written.insert(name);
				}
			}
		}

		// Ensure read local variables has been written to.
		written.clear();
		for (auto activeNode : activeNodes)
		{
			if (const Variable * variableNode = dynamic_type_cast<const Variable*>(activeNode))
			{
				if (variableNode->isGlobal())
					continue;
				if (shaderGraph->findSourcePin(variableNode->findInputPin(L"Input")) == nullptr)
					continue;

				const auto & name = variableNode->getName();
				written.insert(name);
			}
		}
		for (auto activeNode : activeNodes)
		{
			if (const Variable * variableNode = dynamic_type_cast<const Variable*>(activeNode))
			{
				if (variableNode->isGlobal())
					continue;
				if (shaderGraph->getDestinationCount(variableNode->findOutputPin(L"Output")) > 0)
				{
					const auto& name = variableNode->getName();
					if (written.find(name) != written.end())
						outReport.addError(L"Cannot read local variable \"" + name + L"\" as it's not being written to.");
				}
			}
		}
	}
};

class CheckTypes : public Specification
{
public:
	virtual void check(Report& outReport, const ShaderGraph* shaderGraph, const std::set< const Node* >& activeNodes)
	{
		for (auto activeNode : activeNodes)
		{
			const int inputPinCount = activeNode->getInputPinCount();
			for (int i = 0; i < inputPinCount; ++i)
			{
				const Edge* edge = shaderGraph->findEdge(activeNode->getInputPin(i));
				if (!edge)
					continue;

				const Constant value = ShaderGraphEvaluator(shaderGraph).evaluate(edge->getSource());
				if (value.getType() == PinType::Void)
					continue;

				const INodeTraits* traits = INodeTraits::find(activeNode);
				if (!traits)
					continue;

				if (!traits->isInputTypeValid(shaderGraph, activeNode, edge->getDestination(), value.getType()))
					outReport.addError(
						L"Invalid data type into \"" + edge->getDestination()->getName() + L"\".",
						activeNode
					);
			}
		}
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphValidator", ShaderGraphValidator, Object)

ShaderGraphValidator::ShaderGraphValidator(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
}

ShaderGraphValidator::ShaderGraphValidator(const ShaderGraph* shaderGraph, const Guid& shaderGraphId)
:	m_shaderGraph(shaderGraph)
,	m_shaderGraphId(shaderGraphId)
{
}

ShaderGraphValidator::ShaderGraphType ShaderGraphValidator::estimateType() const
{
	if (!m_shaderGraph->findNodesOf< InputPort >().empty())
		return SgtFragment;
	if (!m_shaderGraph->findNodesOf< OutputPort >().empty())
		return SgtFragment;
	if (!m_shaderGraph->findNodesOf< Branch >().empty())
		return SgtFragment;

	return SgtProgram;
}

bool ShaderGraphValidator::validate(ShaderGraphType type, std::vector< const Node* >* outErrorNodes) const
{
	// Collect root nodes.
	RefArray< Node > roots;
	for (auto node : m_shaderGraph->getNodes())
	{
		if (node->getOutputPinCount() <= 0 && node->getInputPinCount() > 0)
			roots.push_back(node);
		else if (is_a< InputPort >(node) || is_a< OutputPort >(node))
			roots.push_back(node);
	}

	// Collect active nodes from root nodes.
	CollectVisitor visitor;
	GraphTraverse(m_shaderGraph, roots).preorder(visitor);

	// Perform checks on active nodes.
	Report report(outErrorNodes);

	EdgeNodes().check(report, m_shaderGraph, visitor.m_nodes);
	UniqueTechniques().check(report, m_shaderGraph, visitor.m_nodes);
	NonOptionalInputs().check(report, m_shaderGraph, visitor.m_nodes);
	SwizzlePatterns().check(report, m_shaderGraph, visitor.m_nodes);
	ParameterNames().check(report, m_shaderGraph, visitor.m_nodes);
	VariableNames().check(report, m_shaderGraph, visitor.m_nodes);
	CheckTypes().check(report, m_shaderGraph, visitor.m_nodes);

	if (type == SgtFragment)
		PortNames().check(report, m_shaderGraph, visitor.m_nodes);
	else if (type == SgtProgram)
	{
		NoPorts().check(report, m_shaderGraph, visitor.m_nodes);
		NoMetaNodes().check(report, m_shaderGraph, visitor.m_nodes);
	}
	else
		NoPorts().check(report, m_shaderGraph, visitor.m_nodes);

	return bool(report.getErrorCount() == 0);
}

bool ShaderGraphValidator::validateIntegrity() const
{
	const RefArray< Node >& nodes = m_shaderGraph->getNodes();

	for (auto edge : m_shaderGraph->getEdges())
	{
		const OutputPin* sourcePin = edge->getSource();
		const InputPin* destinationPin = edge->getDestination();

		if (!sourcePin || !destinationPin)
		{
			log::error << L"Invalid edge found in shader graph (" << m_shaderGraphId.format() << L")." << Endl;
			if (sourcePin)
				log::error << L"Source pin from a " << type_name(sourcePin->getNode()) << L" node (" << sourcePin->getNode()->getId().format() << L"); \"" << sourcePin->getName() << L"\"." << Endl;
			if (destinationPin)
				log::error << L"Destination pin to a " << type_name(destinationPin->getNode()) << L" node (" << destinationPin->getNode()->getId().format() << L"); \"" << destinationPin->getName() << L"\"." << Endl;
			return false;
		}

		if (std::find(nodes.begin(), nodes.end(), sourcePin->getNode()) == nodes.end())
		{
			log::error << L"Source node " << sourcePin->getNode()->getId().format() << L" (" << type_name(sourcePin->getNode()) << L") of edge (pin \"" << sourcePin->getName() << L"\") not part of shader graph (" << m_shaderGraphId.format() << L")." << Endl;
			return false;
		}

		if (std::find(nodes.begin(), nodes.end(), destinationPin->getNode()) == nodes.end())
		{
			log::error << L"Destination node " << destinationPin->getNode()->getId().format() << L" (" << type_name(destinationPin->getNode()) << L") of edge (pin \"" << destinationPin->getName() << L"\") not part of shader graph (" << m_shaderGraphId.format() << L")." << Endl;
			return false;
		}

		if (sourcePin->getNode() == destinationPin->getNode())
		{
			log::error << L"Circular edge; both pins reference same node." << Endl;
			return false;
		}
	}

	for (auto node : nodes)
	{
		SmallSet< Guid > ids;

		const int inputPinCount = node->getInputPinCount();
		for (int i = 0; i < inputPinCount; ++i)
		{
			const Guid& id = node->getInputPin(i)->getId();
			if (ids.find(id) != ids.end())
			{
				log::error << L"Node " << node->getId().format() << L" (" << type_name(node) << L") contain invalid pins; duplicated ids." << Endl;
				return false;
			}
			ids.insert(id);
		}

		const int outputPinCount = node->getOutputPinCount();
		for (int i = 0; i < outputPinCount; ++i)
		{
			const Guid& id = node->getOutputPin(i)->getId();
			if (ids.find(id) != ids.end())
			{
				log::error << L"Node " << node->getId().format() << L" (" << type_name(node) << L") contain invalid pins; duplicated ids." << Endl;
				return false;
			}
			ids.insert(id);
		}
	}

	return true;
}

	}
}
