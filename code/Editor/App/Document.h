#ifndef traktor_editor_Document_H
#define traktor_editor_Document_H

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

	virtual void editInstance(db::Instance* instance, ISerializable* object);

	virtual bool containInstance(db::Instance* instance) const;

	virtual uint32_t getInstanceCount() const;

	virtual db::Instance* getInstance(uint32_t index) const;

	virtual ISerializable* getObject(uint32_t index) const;

	virtual bool setObject(uint32_t index, ISerializable* object);

	virtual void setModified();

	virtual void push();

	virtual bool undo();

	virtual bool redo();

	bool save();

	bool modified() const;

	bool close();

	const RefArray< db::Instance >& getInstances() const { return m_instances; }

private:
	struct HistoryState
	{
		RefArray< ISerializable > objects;
		std::vector< uint32_t > objectHashes;
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

#endif	// traktor_editor_Document_H
