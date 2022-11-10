/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class GraphEvaluator;
class IAudioMixer;
class InputPin;
class ISoundBufferCursor;
class OutputPin;
struct SoundBlock;

/*! Node instance.
 */
class T_DLLCLASS Node : public ISerializable
{
	T_RTTI_CLASS;

public:
	Node();

	/*! Set node instance id.
	 *
	 * \return Instance id.
	 */
	void setId(const Guid& instanceId);

	/*! Get node instance id.
	 *
	 * \return Instance id.
	 */
	const Guid& getId() const;

	/*! Set comment.
	 *
	 * \param comment New comment.
	 */
	void setComment(const std::wstring& comment);

	/*! Get comment.
	 *
	 * \return Comment.
	 */
	const std::wstring& getComment() const;

	/*! Get information.
	 *
	 * \return Information.
	 */
	virtual std::wstring getInformation() const;

	/*! Set position.
	 *
	 * \param position New position.
	 */
	void setPosition(const std::pair< int, int >& position);

	/*! Get position.
	 *
	 * \return Position.
	 */
	const std::pair< int, int >& getPosition() const;

	/*! Get number of input pins.
	 *
	 * \return Number of input pins.
	 */
	virtual size_t getInputPinCount() const = 0;

	/*! Get input pin.
	 *
	 * \param index Index of input pin.
	 * \return Pointer to input pin, null if no such input pin.
	 */
	virtual const InputPin* getInputPin(size_t index) const = 0;

	/*! Get number of output pins.
	 *
	 * \return Number of output pins.
	 */
	virtual size_t getOutputPinCount() const = 0;

	/*! Get output pin.
	 *
	 * \param index Index of output pin.
	 * \return Pointer to output pin, null if no such output pin.
	 */
	virtual const OutputPin* getOutputPin(size_t index) const = 0;

	/*! Find input pin by name.
	 *
	 * \param name Name of input pin.
	 * \return Pointer to input pin, null if no such input pin.
	 */
	const InputPin* findInputPin(const std::wstring& name) const;

	/*! Find output pin by name.
	 *
	 * \param name Name of output pin.
	 * \return Pointer to output pin, null if no such output pin.
	 */
	const OutputPin* findOutputPin(const std::wstring& name) const;

	/*! \brief
	 */
	virtual bool bind(resource::IResourceManager* resourceManager) = 0;

	/*! \brief
	 */
	virtual Ref< ISoundBufferCursor > createCursor() const = 0;

	/*! \brief
	 */
	virtual bool getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const = 0;

	/*! \brief
	 */
	virtual bool getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, SoundBlock& outBlock) const = 0;

	/*! \brief
	 */
	virtual void serialize(ISerializer& s) override;

private:
	Guid m_id;
	std::wstring m_comment;
	std::pair< int, int > m_position;
};

	}
}
