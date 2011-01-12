#include "Amalgam/Editor/ButtonCell.h"
#include "Amalgam/Editor/GraphCell.h"
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

TargetCell::TargetCell(ui::Bitmap* bitmap, TargetInstance* instance)
:	m_instance(instance)
{
	m_progressCell = new ProgressCell();
	m_playCell = new ButtonCell(bitmap, 0, true, ui::EiUser + 1, instance);
	m_stopCell = new ButtonCell(bitmap, 2, false, ui::EiUser + 2, instance);
}

int32_t TargetCell::getHeight() const
{
	const RefArray< TargetConnection >& connections = m_instance->getConnections();
	return 28 + connections.size() * 24;
}

void TargetCell::placeCells(ui::custom::AutoWidget* widget, const ui::Rect& rect)
{
	ui::Rect controlRect = rect; controlRect.bottom = rect.top + 28;

	widget->placeCell(
		m_progressCell,
		ui::Rect(
			controlRect.left + 4,
			controlRect.getCenter().y - 8,
			controlRect.right - 24 * 2 - 8,
			controlRect.getCenter().y + 8
		)
	);

	widget->placeCell(
		m_playCell,
		ui::Rect(
			controlRect.right - 24 * 2 - 4,
			controlRect.top,
			controlRect.right - 24 * 1 - 4,
			controlRect.bottom
		)
	);

	widget->placeCell(
		m_stopCell,
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
	performanceRect.bottom = performanceRect.top + 24;
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
	performanceRect.bottom = performanceRect.top + 24;
	for (uint32_t i = 0; i < connections.size(); ++i)
	{
		const TargetPerformance& performance = connections[i]->getPerformance();

		ui::Rect topRect = performanceRect;
		topRect.bottom = topRect.top + topRect.getHeight() / 2;

		topRect.left += 6;
		canvas.drawText(topRect, toString(int32_t(performance.fps)), ui::AnLeft, ui::AnCenter);

		topRect.left += 30;
		canvas.drawText(topRect, L"U: " + toString(int32_t(performance.update * 1000.0f)) + L" ms", ui::AnLeft, ui::AnCenter);

		topRect.left += 50;
		canvas.drawText(topRect, L"B: " + toString(int32_t(performance.build * 1000.0f)) + L" ms", ui::AnLeft, ui::AnCenter);

		topRect.left += 50;
		canvas.drawText(topRect, L"R: " + toString(int32_t(performance.render * 1000.0f)) + L" ms", ui::AnLeft, ui::AnCenter);

		topRect.left += 50;
		canvas.drawText(topRect, L"p: " + toString(int32_t(performance.physics * 1000.0f)) + L" ms", ui::AnLeft, ui::AnCenter);

		topRect.left += 50;
		canvas.drawText(topRect, L"i: " + toString(int32_t(performance.input * 1000.0f)) + L" ms", ui::AnLeft, ui::AnCenter);

		ui::Rect bottomRect = performanceRect;
		bottomRect.top = bottomRect.top + bottomRect.getHeight() / 2;

		bottomRect.left += 6;
		canvas.drawText(bottomRect, L"M: " + toString(uint32_t(performance.memInUse / 1024)) + L" KiB", ui::AnLeft, ui::AnCenter);

		bottomRect.left += 80;
		canvas.drawText(bottomRect, L"O: " + toString(uint32_t(performance.heapObjects)), ui::AnLeft, ui::AnCenter);

		performanceRect = performanceRect.offset(0, performanceRect.getHeight());
	}

	canvas.setFont(widgetFont);

	switch (m_instance->getState())
	{
	case TsIdle:
		m_playCell->setEnable(true);
		m_stopCell->setEnable(false);
		break;

	case TsPending:
		m_playCell->setEnable(false);
		m_stopCell->setEnable(false);
		break;

	case TsBuilding:
		m_playCell->setEnable(false);
		m_stopCell->setEnable(true);
		break;

	case TsDeploying:
		m_playCell->setEnable(false);
		m_stopCell->setEnable(false);
		break;

	case TsLaunching:
		m_playCell->setEnable(false);
		m_stopCell->setEnable(false);
		break;
	}

	// Re-issue update; we need to refresh continiously.
	widget->requestLayout();
	widget->requestUpdate();
}

	}
}
