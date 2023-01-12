/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/GraphTraverse.h"

namespace traktor::render
{
	namespace
	{

struct FindInputPin
{
	const InputPin* inputPin = nullptr;
	bool found = false;

	bool operator () (Node* node)
	{
		found |= bool(inputPin->getNode() == node);
		return !found;
	}

	bool operator () (Edge* edge)
	{
		return !found;
	}
};

struct PinsConnected
{
	const InputPin* inputPin = nullptr;
	const OutputPin* outputPin = nullptr;
	bool connected = false;

	bool operator () (Node* node)
	{
		return !connected;
	}

	bool operator () (Edge* edge)
	{
		// Don't traverse paths from source node from wrong input pin.
		if (
			edge->getDestination()->getNode() == inputPin->getNode() &&
			edge->getDestination() != inputPin
		)
			return false;

		connected |= bool(outputPin == edge->getSource());
		return true;
	}
};

struct CollectOutputs
{
	const InputPin* inputPin = nullptr;
	AlignedVector< const OutputPin* > outputPins;

	bool operator () (Node* node)
	{
		return true;
	}

	bool operator () (Edge* edge)
	{
		// Don't traverse paths from source node from wrong input pin.
		if (
			edge->getDestination()->getNode() == inputPin->getNode() &&
			edge->getDestination() != inputPin
		)
			return false;

		outputPins.push_back(edge->getSource());
		return true;
	}
};

	}

bool doesInputPropagateToNode(const Graph* graph, const InputPin* inputPin, Node* targetNode)
{
	FindInputPin visitor;
	visitor.inputPin = inputPin;
	visitor.found = false;
	GraphTraverse(graph, targetNode).preorder(visitor);
	return visitor.found;
}

bool arePinsConnected(const Graph* graph, const OutputPin* outputPin, const InputPin* inputPin)
{
	PinsConnected visitor;
	visitor.outputPin = outputPin;
	visitor.inputPin = inputPin;
	visitor.connected = false;
	GraphTraverse(graph, inputPin->getNode()).preorder(visitor);
	return visitor.connected;
}

void getMergingOutputs(const Graph* graph, const AlignedVector< const InputPin* >& inputPins, AlignedVector< const OutputPin* >& outMergedOutputPins)
{
	T_ASSERT(inputPins.size() >= 2);

	// Collect all reachable output pins.
	AlignedVector< CollectOutputs > visitors(inputPins.size());
	for (size_t i = 0; i < inputPins.size(); ++i)
	{
		visitors[i].inputPin = inputPins[i];
		GraphTraverse(graph, inputPins[i]->getNode()).preorder(visitors[i]);
	}

	// Keep only output pins which are found from all inputs.
	AlignedVector< const OutputPin* >& commonOutputPins = visitors[0].outputPins;
	for (size_t i = 1; i < visitors.size(); ++i)
	{
		for (size_t j = 0; j < commonOutputPins.size(); )
		{
			if (std::find(visitors[i].outputPins.begin(), visitors[i].outputPins.end(), commonOutputPins[j]) == visitors[i].outputPins.end())
				commonOutputPins.erase(commonOutputPins.begin() + j);
			else
				++j;
		}
	}

	// Keep only "right most" output pins.
	for (size_t i = 0; i < commonOutputPins.size(); ++i)
	{
		bool connected = false;

		for (size_t j = 0; j < commonOutputPins.size() && !connected; ++j)
		{
			if (i == j)
				continue;

			const Node* checkNode = commonOutputPins[j]->getNode();
			const int32_t checkInputPinCount = checkNode->getInputPinCount();
			for (int32_t k = 0; k < checkInputPinCount && !connected; ++k)
			{
				const InputPin* checkInputPin = checkNode->getInputPin(k);
				connected = arePinsConnected(graph, commonOutputPins[i], checkInputPin);
			}
		}

		if (!connected)
			outMergedOutputPins.push_back(commonOutputPins[i]);
	}
}

void getNonDependentOutputs(const Graph* graph, const InputPin* inputPin, const AlignedVector< const OutputPin* >& dependentOutputPins, AlignedVector< const OutputPin* >& outOutputPins)
{
	CollectOutputs visitor;
	visitor.inputPin = inputPin;
	GraphTraverse(graph, inputPin->getNode()).preorder(visitor);

	// Keep only output pins which are not dependent on input from dependentOutputPins.
	AlignedVector< const OutputPin* > nonDependentOutputPins;
    for (auto outputPin : visitor.outputPins)
	{
		bool outputNodeDependent = false;

		Node* outputNode = outputPin->getNode();
		const int32_t outputNodeInputPinCount = outputNode->getInputPinCount();
		for (int32_t j = 0; j < outputNodeInputPinCount && !outputNodeDependent; ++j)
		{
			const InputPin* outputNodeInputPin = outputNode->getInputPin(j);
            for (auto dependentOutputPin : dependentOutputPins)
			{
				if (arePinsConnected(graph, dependentOutputPin, outputNodeInputPin))
					outputNodeDependent = true;
			}
		}

		if (!outputNodeDependent)
			nonDependentOutputPins.push_back(outputPin);
	}

	// Keep only "right most" output pins.
	for (size_t i = 0; i < nonDependentOutputPins.size(); ++i)
	{
		bool connected = false;

		for (size_t j = 0; j < nonDependentOutputPins.size() && !connected; ++j)
		{
			if (i == j)
				continue;

			const Node* checkNode = nonDependentOutputPins[j]->getNode();
			const int32_t checkInputPinCount = checkNode->getInputPinCount();
			for (int32_t k = 0; k < checkInputPinCount && !connected; ++k)
			{
				const InputPin* checkInputPin = checkNode->getInputPin(k);
				connected = arePinsConnected(graph, nonDependentOutputPins[i], checkInputPin);
			}
		}

		if (!connected)
			outOutputPins.push_back(nonDependentOutputPins[i]);
	}
}

}
