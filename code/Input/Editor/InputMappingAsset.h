/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class IInputNode;
class InputMappingSourceData;
class InputMappingStateData;

/*! Input mapping asset.
 * \ingroup Input
 */
class T_DLLCLASS InputMappingAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Position
	{
		int32_t x;
		int32_t y;
	};

	void addInputNode(IInputNode* inputNode);

	void removeInputNode(IInputNode* inputNode);

	const RefArray< IInputNode >& getInputNodes() const;

	void setPosition(const Object* object, const Position& position);

	Position getPosition(const Object* object) const;

	virtual void serialize(ISerializer& s) override final;

	void setSourceData(InputMappingSourceData* sourceData) { m_sourceData = sourceData; }

	InputMappingSourceData* getSourceData() { return m_sourceData; }

	const InputMappingSourceData* getSourceData() const { return m_sourceData; }

	void setStateData(InputMappingStateData* stateData)  { m_stateData = stateData; }

	InputMappingStateData* getStateData() { return m_stateData; }

	const InputMappingStateData* getStateData() const { return m_stateData; }

	const std::list< Guid >& getDependencies() const { return m_dependencies; }

private:
	struct ObjectPosition
	{
		Ref< const Object > object;
		Position position;

		ObjectPosition();

		ObjectPosition(const Object* object, const Position& position);

		void serialize(ISerializer& s);
	};

	RefArray< IInputNode > m_inputNodes;
	AlignedVector< ObjectPosition > m_positions;
	Ref< InputMappingSourceData > m_sourceData;
	Ref< InputMappingStateData > m_stateData;
	std::list< Guid > m_dependencies;
};

	}
}

