#include <iomanip>
#include "Amalgam/Editor/ButtonCell.h"
#include "Amalgam/Editor/DropListCell.h"
#include "Amalgam/Editor/GraphCell.h"
#include "Amalgam/Editor/HostEnumerator.h"
#include "Amalgam/Editor/ProgressCell.h"
#include "Amalgam/Editor/TargetCell.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Custom/Auto/AutoWidget.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const int32_t c_performanceHeight = 48;

std::wstring formatPerformanceTime(float time)
{
	std::wstringstream ss;
	ss << std::setprecision(1) << std::fixed << (time * 1000.0f);
	return ss.str();
}

std::wstring formatPerformanceValue(float value)
{
	std::wstringstream ss;
	ss << std::setprecision(1) << std::fixed << value;
	return ss.str();
}

		}

TargetCell::TargetCell(ui::Bitmap* bitmap, HostEnumerator* hostEnumerator, TargetInstance* instance)
:	m_instance(instance)
{
	m_progressCell = new ProgressCell();
	m_hostsCell = new DropListCell(hostEnumerator, instance);
	m_playCell = new ButtonCell(bitmap, 0, true, ui::EiUser + 1, instance);
}

int32_t TargetCell::getHeight() const
{
	const RefArray< TargetConnection >& connections = m_instance->getConnections();
	return 28 + connections.size() * c_performanceHeight;
}

void TargetCell::placeCells(ui::custom::AutoWidget* widget, const ui::Rect& rect)
{
	ui::Rect controlRect = rect; controlRect.bottom = rect.top + 28;

	if (m_instance->getState() != TsIdle)
	{
		widget->placeCell(
			m_progressCell,
			ui::Rect(
				controlRect.left + 4,
				controlRect.getCenter().y - 8,
				controlRect.right - 24 * 1 - 8,
				controlRect.getCenter().y + 8
			)
		);
	}

	if (m_instance->getState() == TsIdle)
	{
		widget->placeCell(
			m_hostsCell,
			ui::Rect(
				controlRect.getCenter().x - 16,
				controlRect.getCenter().y - 10,
				controlRect.right - 24 * 1 - 12,
				controlRect.getCenter().y + 10
			)
		);
	}

	widget->placeCell(
		m_playCell,
		ui::Rect(
			controlRect.right - 24 * 1 - 4,
			controlRect.top,
			controlRect.right - 24 * 0 - 4,
			controlRect.bottom
		)
	);
}

void TargetCell::paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect)
{
	const RefArray< TargetConnection >& connections = m_instance->getConnections();

	ui::Rect controlRect = rect; controlRect.bottom = rect.top + 28;

	canvas.setForeground(Color4ub(255, 255, 255));
	canvas.setBackground(ui::getSystemColor(ui::ScButtonFace));
	canvas.fillGradientRect(controlRect);

	ui::Rect performanceRect = rect;
	performanceRect.top = rect.top + 28;
	performanceRect.bottom = performanceRect.top + c_performanceHeight;
	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		canvas.setForeground(Color4ub(180, 180, 180));
		canvas.setBackground(Color4ub(200, 200, 200));
		canvas.fillGradientRect(performanceRect);
		performanceRect = performanceRect.offset(0, performanceRect.getHeight());
	}

	canvas.setForeground(ui::getSystemColor(ui::ScButtonShadow));
	canvas.drawLine(rect.left, rect.bottom - 1, rect.right, rect.bottom - 1);

	if (m_instance->getState() == TsBuilding)
	{
		int32_t progress = m_instance->getBuildProgress();
		m_progressCell->setProgress(progress);
	}
	else
		m_progressCell->setProgress(-1);

	ui::Rect textRect = controlRect;
	textRect.left += 6;
	textRect.right -= 24 * 3 - 8;

	canvas.setForeground(ui::getSystemColor(ui::ScWindowText));
	canvas.drawText(textRect, m_instance->getName(), ui::AnLeft, ui::AnCenter);

	ui::Font widgetFont = widget->getFont();
	ui::Font performanceFont = widgetFont; performanceFont.setSize(8);
	canvas.setFont(performanceFont);

	performanceRect = rect;
	performanceRect.top = rect.top + 28;
	performanceRect.bottom = performanceRect.top + c_performanceHeight;
	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		const TargetPerformance& performance = connections[i]->getPerformance();
		const TargetPerformance& deltaPerformance = connections[i]->getDeltaPerformance();

		ui::Rect topRect = performanceRect;
		topRect.bottom = topRect.top + 12;

		topRect.left += 6;
		canvas.drawText(topRect, toString(int32_t(performance.fps)), ui::AnLeft, ui::AnCenter);

		topRect.left += 20;
		canvas.drawText(topRect, L"U: " + formatPerformanceTime(performance.update) + L" (" + formatPerformanceTime(deltaPerformance.update) + L")", ui::AnLeft, ui::AnCenter);

		topRect.left += 80;
		canvas.drawText(topRect, L"B: " + formatPerformanceTime(performance.build) + L" (" + formatPerformanceTime(deltaPerformance.build) + L")", ui::AnLeft, ui::AnCenter);

		topRect.left += 80;
		canvas.drawText(topRect, L"R: " + formatPerformanceTime(performance.render) + L" (" + formatPerformanceTime(deltaPerformance.render) + L")", ui::AnLeft, ui::AnCenter);

		ui::Rect middleRect = performanceRect;
		middleRect.top = performanceRect.top + 12;
		middleRect.bottom = performanceRect.top + 24;

		middleRect.left += 26;
		canvas.drawText(middleRect, L"P: " + formatPerformanceTime(performance.physics) + L" (" + formatPerformanceTime(deltaPerformance.physics) + L")", ui::AnLeft, ui::AnCenter);

		middleRect.left += 80;
		canvas.drawText(middleRect, L"I: " + formatPerformanceTime(performance.input) + L" (" + formatPerformanceTime(deltaPerformance.input) + L")", ui::AnLeft, ui::AnCenter);

		middleRect.left += 80;
		canvas.drawText(middleRect, L"S: " + toString(int32_t(performance.steps)) + L", " + formatPerformanceTime(performance.interval) + L", " + toString(performance.collisions), ui::AnLeft, ui::AnCenter);

		ui::Rect middleRect2 = performanceRect;
		middleRect2.top = performanceRect.top + 24;
		middleRect2.bottom = performanceRect.top + 36;

		middleRect2.left += 26;
		canvas.drawText(middleRect2, L"Draw: " + toString(int32_t(performance.drawCalls)), ui::AnLeft, ui::AnCenter);

		middleRect2.left += 100;
		canvas.drawText(middleRect2, L"Prim: " + toString(int32_t(performance.primitiveCount)), ui::AnLeft, ui::AnCenter);

		ui::Rect bottomRect = performanceRect;
		bottomRect.top = performanceRect.top + 36;

		bottomRect.left += 26;
		canvas.drawText(bottomRect, L"Mem: " + toString(uint32_t(performance.memInUse / 1024)) + L" KiB", ui::AnLeft, ui::AnCenter);

		bottomRect.left += 100;
		canvas.drawText(bottomRect, L"Obj: " + toString(uint32_t(performance.heapObjects)), ui::AnLeft, ui::AnCenter);

		performanceRect = performanceRect.offset(0, performanceRect.getHeight());
	}

	canvas.setFont(widgetFont);

	m_playCell->setEnable(m_instance->getState() == TsIdle);

	// Re-issue update; we need to refresh continiously if we're either running or building.
	if (m_instance->getState() != TsIdle || !m_instance->getConnections().empty())
	{
		widget->requestLayout();
		widget->requestUpdate();
	}
}

	}
}
