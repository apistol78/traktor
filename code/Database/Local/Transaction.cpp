#include "Database/Local/Transaction.h"
#include "Database/Local/Action.h"
#include "Core/Heap/New.h"
#include "Core/Thread/Mutex.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

const int c_transactionTimeout = 100;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Transaction", Transaction, Object)

bool Transaction::create(const Guid& transactionGuid)
{
	m_lock = gc_new< Mutex >(cref(transactionGuid));
	if (!m_lock->acquire(c_transactionTimeout))
	{
		log::debug << L"Unable to create transaction; already exclusively locked" << Endl;
		m_lock->release();
		m_lock = 0;
		return false;
	}
	return true;
}

void Transaction::destroy()
{
	if (m_lock)
	{
		m_lock->release();
		m_lock = 0;
	}
}

void Transaction::add(Action* action)
{
	m_actions.push_back(action);
}

bool Transaction::commit(Context* context)
{
	bool result = true;

	// Execute actions.
	for (int32_t i = 0; i < int32_t(m_actions.size()); ++i)
	{
		if (!m_actions[i]->execute(context))
		{
			for (int32_t j = i; j >= 0; --j)
			{
				if (!m_actions[j]->undo(context))
					log::error << L"Unable to undo transaction step " << j << L"; data integrity might be corrupt" << Endl;
			}
			result = false;
			break;
		}
	}

	// Cleanup action temporary data.
	for (int32_t i = 0; i < int32_t(m_actions.size()); ++i)
		m_actions[i]->clean(context);

	return result;
}

	}
}
