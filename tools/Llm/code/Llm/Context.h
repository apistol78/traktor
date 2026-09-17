/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_LLM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::llm
{

class Model;

/*! Context length used when the caller does not choose one.
 *
 * Well short of what a modern model is trained for, on purpose: the key/value
 * cache grows linearly with this, and a 3B model at a 32k context wants over
 * two gigabytes of it before generating a thing.
 */
constexpr int32_t c_defaultContextLength = 4096;

/*! Evaluation state for one conversation.
 * \ingroup Llm
 *
 * Holds the key/value cache and the scratch buffers of a single forward
 * pass. One context is one sequence; run two conversations at once by
 * creating two contexts over the same model.
 *
 * Not thread safe. Evaluation must not run on a job worker, since it forks
 * work onto the job manager itself.
 */
class T_DLLCLASS Context : public Object
{
	T_RTTI_CLASS;

public:
	/*! Allocate state for \a contextLength positions.
	 *
	 * \param contextLength Zero uses the length the model was trained with.
	 */
	bool create(const Model* model, int32_t contextLength);

	void destroy();

	/*! Forget everything generated so far, keeping the allocation. */
	void reset();

	/*! Drop everything at or after \a position, keeping the prefix.
	 *
	 * Lets a conversation reuse the cache it already built for the turns it
	 * shares with the previous prompt, instead of re-reading them.
	 */
	void rewind(int32_t position);

	/*! Run \a token through the network at the next free position.
	 *
	 * \param computeLogits Skip the output projection when only the key and
	 *                      value cache matter, as when consuming a prompt.
	 *                      Saves the single largest matrix product per token.
	 * \return False if the context is full.
	 */
	bool evaluate(int32_t token, bool computeLogits);

	/*! Logits of the last evaluation, or null if none were computed. */
	float* getLogits() { return m_logitsValid ? m_logits.ptr() : nullptr; }

	int32_t getLogitCount() const;

	int32_t getContextLength() const { return m_contextLength; }

	/*! Number of positions consumed so far. */
	int32_t getPosition() const { return m_position; }

	/*! Bytes held by the key/value cache. */
	int64_t getCacheSize() const;

private:
	Ref< const Model > m_model;
	int32_t m_contextLength = 0;
	int32_t m_position = 0;
	bool m_logitsValid = false;

	AlignedVector< float > m_x;		  //!< Residual stream.
	AlignedVector< float > m_xb;	  //!< Normalized input to a sub layer.
	AlignedVector< float > m_xb2;	  //!< Sub layer output, before the residual add.
	AlignedVector< float > m_hb;	  //!< Feed forward gate branch.
	AlignedVector< float > m_hb2;	  //!< Feed forward up branch.
	AlignedVector< float > m_query;
	AlignedVector< float > m_attention; //!< headCount rows of contextLength scores.
	AlignedVector< float > m_logits;
	AlignedVector< float > m_keyCache;	 //!< layer major, then position, then channel.
	AlignedVector< float > m_valueCache;

	void attend(int32_t layer, int32_t position);
};

}
