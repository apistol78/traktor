#include <cctype>
#include <algorithm>
#include <stack>
#include <map>
#include "Render/Editor/Shader/ShaderGraphValidator.h"
#include "Render/ShaderGraph.h"
#include "Render/ShaderGraphAdjacency.h"
#include "Render/Edge.h"
#include "Render/Nodes.h"
#include "Render/InputPin.h"
#include "Render/OutputPin.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

void collectActiveNodes(const ShaderGraph* shaderGraph, std::set< const Node* >& outActiveNodes)
{
	std::stack< const Node* > nodeStack;

	const RefArray< Node >& nodes = shaderGraph->getNodes();
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		if (is_a< VertexOutput >(*i) || is_a< PixelOutput >(*i) || is_a< OutputPort >(*i))
			nodeStack.push(*i);
	}

	ShaderGraphAdjacency shaderGraphAdj(shaderGraph);

	while (!nodeStack.empty())
	{
		const Node* node = nodeStack.top();
		nodeStack.pop();

		outActiveNodes.insert(node);

		int inputPinCount = node->getInputPinCount();
		for (int i = 0; i < inputPinCount; ++i)
		{
			const InputPin* inputPin = node->getInputPin(i);
			const OutputPin* outputPin = shaderGraphAdj.findSourcePin(inputPin);
			if (outputPin)
			{
				const Node* sourceNode = outputPin->getNode();
				if (outActiveNodes.find(sourceNode) == outActiveNodes.end())
					nodeStack.push(sourceNode);
			}
		}
	}
}

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

		const RefArray< Node >& nodes = shaderGraph->getNodes();
		for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			if (PixelOutput* pixelOutput = dynamic_type_cast< PixelOutput* >(*i))
			{
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
		ShaderGraphAdjacency shaderGraphAdj(shaderGraph);
		for (std::set< const Node* >::const_iterator i = activeNodes.begin(); i != activeNodes.end(); ++i)
		{
			int inputPinCount = (*i)->getInputPinCount();
			for (int j = 0; j < inputPinCount; ++j)
			{
				const InputPin* inputPin = (*i)->getInputPin(j);
				if (!inputPin->isOptional() && !shaderGraphAdj.findSourcePin(inputPin))
					outReport.addError(L"Input pin \"" + inputPin->getName() + L"\" not connected", *i);
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

			if (is_a< Sampler >(*i))
				parameterName = static_cast< const Sampler* >(*i)->getParameterName();
			else if (is_a< Uniform >(*i))
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
		for (std::set< const Node* >::const_iterator i = activeNodes.begin(); i != activeNodes.end(); ++i)
		{
			std::set< std::wstring >* usedNames;
			std::wstring portName;

			if (is_a< InputPort >(*i))
			{
				usedNames = &usedInputNames;
				portName = static_cast< const InputPort* >(*i)->getName();
			}
			else if (is_a< OutputPort >(*i))
			{
				usedNames = &usedOutputNames;
				portName = static_cast< const OutputPort* >(*i)->getName();
			}
			else
				continue;

			if (portName.empty())
				outReport.addError(L"Invalid port name, no name", *i);
			else
			{
				if (usedNames->find(portName) != usedNames->end())
					outReport.addError(L"Port name already in use", *i);
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderGraphValidator", ShaderGraphValidator, Object)

ShaderGraphValidator::ShaderGraphValidator(const ShaderGraph* shaderGraph)
:	m_shaderGraph(shaderGraph)
{
}

bool ShaderGraphValidator::validate(ShaderGraphType type, std::vector< const Node* >* outErrorNodes) const
{
	Report report(outErrorNodes);

	std::set< const Node* > activeNodes;
	collectActiveNodes(m_shaderGraph, activeNodes);

	EdgeNodes().check(report, m_shaderGraph, activeNodes);
	UniqueTechniques().check(report, m_shaderGraph, activeNodes);
	NonOptionalInputs().check(report, m_shaderGraph, activeNodes);
	SwizzlePatterns().check(report, m_shaderGraph, activeNodes);
	ParameterNames().check(report, m_shaderGraph, activeNodes);
	if (type == SgtFragment)
		PortNames().check(report, m_shaderGraph, activeNodes);
	else
	{
		NoPorts().check(report, m_shaderGraph, activeNodes);
		if (type == SgtProgram)
			NoMetaNodes().check(report, m_shaderGraph, activeNodes);
	}

	return bool(report.getErrorCount() == 0);
}

	}
}
