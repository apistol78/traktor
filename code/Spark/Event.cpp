#include "Core/Class/Any.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Spark/Event.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Event", Event, Object)

IRuntimeDelegate* Event::add(IRuntimeDelegate* rd)
{
	m_rds.push_back(rd);
	return rd;
}

void Event::remove(IRuntimeDelegate* rd)
{
	m_rds.remove(rd);
}

void Event::removeAll()
{
	m_rds.clear();
}

bool Event::empty() const
{
	return m_rds.empty();
}

void Event::issue()
{
	for (auto rd : m_rds)
		rd->call(0, nullptr);
}

void Event::issue(int32_t argc, const Any* argv)
{
	for (auto rd : m_rds)
		rd->call(argc, argv);
}

	}
}
