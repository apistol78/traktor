#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Serialization/DeepHash.h"
#include "Database/Instance.h"
#include "Editor/App/Document.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.Document", Document, IDocument)

Document::Document()
:	m_modified(false)
{
}

Document::~Document()
{
	T_ASSERT_M (m_instances.empty(), L"Document not closed properly");
}

void Document::editInstance(db::Instance* instance, ISerializable* object)
{
	RefArray< db::Instance >::iterator i = std::find(m_instances.begin(), m_instances.end(), instance);
	if (i != m_instances.end())
	{
		uint32_t index = std::distance(m_instances.begin(), i);
		setObject(index, object);
		return;
	}

	m_instances.push_back(instance);
	m_objects.push_back(object);
	m_objectHashes.push_back(DeepHash(object).get());
}

bool Document::containInstance(db::Instance* instance) const
{
	RefArray< db::Instance >::const_iterator i = std::find(m_instances.begin(), m_instances.end(), instance);
	return i != m_instances.end();
}

uint32_t Document::getInstanceCount() const
{
	return uint32_t(m_instances.size());
}

db::Instance* Document::getInstance(uint32_t index) const
{
	return index < uint32_t(m_instances.size()) ? m_instances[index] : 0;
}

ISerializable* Document::getObject(uint32_t index) const
{
	return index < uint32_t(m_objects.size()) ? m_objects[index] : 0;
}

bool Document::setObject(uint32_t index, ISerializable* object)
{
	if (index >= uint32_t(m_objects.size()))
		return false;

	m_objects[index] = object;
	return true;
}

void Document::setModified()
{
	m_modified = true;
}

void Document::push()
{
	if (!m_undoHistory.empty())
	{
		const std::vector< uint32_t >& lastHashes = m_undoHistory.back().objectHashes;
		if (lastHashes.size() == m_objects.size())
		{
			bool eq = true;
			for (uint32_t i = 0; i < lastHashes.size(); ++i)
			{
				if (DeepHash(m_objects[i]).get() != lastHashes[i])
				{
					eq = false;
					break;
				}
			}
		}
	}

	HistoryState state;

	state.objects.resize(m_objects.size());
	state.objectHashes.resize(m_objectHashes.size());

	for (uint32_t i = 0; i < m_objects.size(); ++i)
	{
		state.objects[i] = DeepClone(m_objects[i]).create();
		state.objectHashes[i] = m_objectHashes[i];
	}

	m_undoHistory.push_back(state);
	m_redoHistory.clear();
}

bool Document::undo()
{
	if (m_undoHistory.empty())
		return false;

	HistoryState redoState;

	redoState.objects.resize(m_objects.size());
	redoState.objectHashes.resize(m_objectHashes.size());

	for (uint32_t i = 0; i < m_objects.size(); ++i)
	{
		redoState.objects[i] = DeepClone(m_objects[i]).create();
		redoState.objectHashes[i] = m_objectHashes[i];
	}

	m_redoHistory.push_back(redoState);

	HistoryState& undoState = m_undoHistory.back();
	for (uint32_t i = 0; i < undoState.objects.size(); ++i)
		m_objects[i] = undoState.objects[i];
	
	m_undoHistory.pop_back();

	return true;
}

bool Document::redo()
{
	if (m_redoHistory.empty())
		return false;

	HistoryState& redoState = m_redoHistory.back();
	for (uint32_t i = 0; i < redoState.objects.size(); ++i)
		m_objects[i] = redoState.objects[i];

	m_redoHistory.pop_back();

	return true;
}

bool Document::save()
{
	for (uint32_t i = 0; i < m_instances.size(); ++i)
	{
		if (m_objects[i] && !m_instances[i]->setObject(m_objects[i]))
		{
			log::error << L"Unable to save document; failed to set instance object" << Endl;
			return false;
		}

		if (!m_instances[i]->commit(db::CfKeepCheckedOut))
		{
			log::error << L"Unable to save document; failed to commit instance" << Endl;
			return false;
		}

		m_objectHashes[i] = DeepHash(m_objects[i]).get();
	}

	m_modified = false;
	return true;
}

bool Document::modified() const
{
	if (m_modified)
		return true;

	for (uint32_t i = 0; i < m_objects.size(); ++i)
	{
		if (DeepHash(m_objects[i]).get() != m_objectHashes[i])
			return true;
	}

	return false;
}

bool Document::close()
{
	for (RefArray< db::Instance >::iterator i = m_instances.begin(); i != m_instances.end(); ++i)
		(*i)->revert();

	m_instances.clear();
	m_objects.clear();
	m_objectHashes.clear();
	m_undoHistory.clear();

	return true;
}

	}
}
