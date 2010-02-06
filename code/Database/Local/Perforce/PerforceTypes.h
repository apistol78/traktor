#ifndef traktor_db_PerforceTypes_H
#define traktor_db_PerforceTypes_H

namespace traktor
{
	namespace db
	{

/*! \brief P4 actions.
 * \ingroup Database
 */
enum PerforceAction
{
	AtNotOpened,
	AtAdd,
	AtEdit,
	AtDelete
};

	}
}

#endif	// traktor_db_PerforceTypes_H
