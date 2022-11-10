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
#include <vector>
#include "Core/RefArray.h"
#include "Editor/IDocument.h"

namespace traktor
{
	namespace editor
	{

class Document : public IDocument
{
	T_RTTI_CLASS;

public:
	Document();

	virtual ~Document();

	virtual void editInstance(db::Instance* instance, ISerializable* object) override final;

	virtual bool containInstance(db::Instance* instance) const override final;

	virtual uint32_t getInstanceCount() const override final;

	virtual db::Instance* getInstance(uint32_t index) const override final;

	virtual ISerializable* getObject(uint32_t index) const override final;

	virtual bool setObject(uint32_t index, ISerializable* object) override final;

	virtual void setModified() override final;

	virtual void push(const ISerializable* meta) override final;

	virtual bool undo(const ISerializable* redoMeta, Ref< const ISerializable >* outMeta) override final;

	virtual bool redo(Ref< const ISerializable >* outMeta) override final;

	bool replaceInstance(uint32_t index, db::Instance* instance);

	bool save();

	bool modified() const;

	bool close();

	const RefArray< db::Instance >& getInstances() const { return m_instances; }

private:
	struct HistoryState
	{
		RefArray< ISerializable > objects;
		std::vector< uint32_t > objectHashes;
		Ref< const ISerializable > meta;
	};

	RefArray< db::Instance > m_instances;
	RefArray< ISerializable > m_objects;
	std::vector< uint32_t > m_objectHashes;
	std::list< HistoryState > m_undoHistory;
	std::list< HistoryState > m_redoHistory;
	bool m_modified;
};

	}
}

