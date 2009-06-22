#include "Database/Local/Transaction.h"
#include "Database/Local/Action.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Transaction", Transaction, Object)

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
