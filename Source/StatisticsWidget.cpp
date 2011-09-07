
#include "StatisticsWidget.h"
#include "MainWindow.h"
#include "Statistics.h"
#include "RenderThread.h"

#define MB 1024.0f * 1024.0f

QStatisticsWidget::QStatisticsWidget(QWidget* pParent) :
	QWidget(pParent),
	m_MainLayout(),
	m_Group(),
	m_Tree()
{
	// Set the size policy, making sure the widget fits nicely in the layout
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	// Status and tooltip
	setToolTip("Statistics");
	setStatusTip("Statistics");

	// Set the main layout
	setLayout(&m_MainLayout);

	// Add the group
	m_MainLayout.addWidget(&m_Group);

	// Set the layout
	m_Group.setLayout(&m_GroupLayout);

	// Add the tree widget
	m_GroupLayout.addWidget(&m_Tree);

	// Configure tree
	m_Tree.setColumnCount(3);

	QStringList ColumnNames;

	ColumnNames << "Property" << "Value" << "Unit";

	m_Tree.setHeaderLabels(ColumnNames);

	// Configure headers
	m_Tree.header()->setResizeMode(0, QHeaderView::ResizeToContents);
//	m_Tree.header()->setResizeMode(1, QHeaderView::ResizeToContents);
//	m_Tree.header()->setResizeMode(2, QHeaderView::ResizeToContents);
	m_Tree.header()->setWindowIcon(QIcon(":/Images/table-export.png"));
	
	PopulateTree();
	
	// Notify us when rendering begins and ends, and before/after each rendered frame
	connect(&gRenderStatus, SIGNAL(RenderBegin()), this, SLOT(OnRenderBegin()));
	connect(&gRenderStatus, SIGNAL(RenderEnd()), this, SLOT(OnRenderEnd()));
	connect(&gRenderStatus, SIGNAL(PreRenderFrame()), this, SLOT(OnPreRenderFrame()));
	connect(&gRenderStatus, SIGNAL(PostRenderFrame()), this, SLOT(OnPostRenderFrame()));
	connect(&gRenderStatus, SIGNAL(StatisticChanged(const QString&, const QString&, const QString&, const QString&, const QString&)), this, SLOT(OnStatisticChanged(const QString&, const QString&, const QString&, const QString&, const QString&)));
}

QSize QStatisticsWidget::sizeHint() const
{
	return QSize(500, 900);
}

void QStatisticsWidget::PopulateTree(void)
{
	// Performance
	QTreeWidgetItem* pPerformance = AddItem(NULL, "Performance");
	pPerformance->setIcon(0, QIcon(":/Images/alarm-clock.png"));

	// Volume
	QTreeWidgetItem* pVolume = AddItem(NULL, "Volume");
	pVolume->setIcon(0, QIcon(":/Images/grid.png"));

	// CUDA Memory
	QTreeWidgetItem* pMemoryCuda = AddItem(NULL, "Memory [CUDA]");
	pMemoryCuda->setIcon(0, QIcon(":/Images/memory.png"));

	// Host Memory
	QTreeWidgetItem* pMemoryHost = AddItem(NULL, "Memory [Host]");
	pMemoryHost->setIcon(0, QIcon(":/Images/memory.png"));

	// Camera
	QTreeWidgetItem* pCamera = AddItem(NULL, "Camera");
	pCamera->setIcon(0, QIcon(":/Images/camera.png"));

	// Camera
	QTreeWidgetItem* pGraphicsCard = AddItem(NULL, "Graphics Card");
	pGraphicsCard->setIcon(0, QIcon(":/Images/graphic-card.png"));
}

QTreeWidgetItem* QStatisticsWidget::AddItem(QTreeWidgetItem* pParent, const QString& Property, const QString& Value, const QString& Unit, const QString& Icon)
{
	// Create new item
	QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent);
	pItem->setText(0, Property);
	pItem->setText(1, Value);
	pItem->setText(2, Unit);

	if (!pParent)
		m_Tree.addTopLevelItem(pItem);

	return pItem;
}

void QStatisticsWidget::UpdateStatistic(const QString& Group, const QString& Name, const QString& Value, const QString& Unit, const QString& Icon)
{
	QTreeWidgetItem* pGroup = FindItem(Group);

	if (!pGroup)
	{
		pGroup = AddItem(NULL, Group);
		pGroup->setIcon(0, QIcon(Icon));
		
		QFont Font = pGroup->font(0);
		Font.setBold(true);

		pGroup->setFont(0, Font);
		pGroup->setBackgroundColor(0, Qt::red);
		
		AddItem(pGroup, Name, Value, Unit, Icon);
	}
	else
	{
		bool Found = false;

		for (int i = 0; i < pGroup->childCount(); i++)
		{
			if (pGroup->child(i)->text(0) == Name)
			{
				pGroup->child(i)->setText(1, Value);
				pGroup->child(i)->setText(2, Unit);
				Found = true;
			}
		}

		if (!Found)
			AddItem(pGroup, Name, Value, Unit, Icon);
	}
}

void QStatisticsWidget::OnRenderBegin(void)
{
	// Expand all tree items
	ExpandAll(true);
}

void QStatisticsWidget::OnRenderEnd(void)
{
	// Collapse all tree items
	ExpandAll(false);

	// Remove 2nd order children
	RemoveChildren("Performance");
	RemoveChildren("Volume");
	RemoveChildren("Memory [CUDA]");
	RemoveChildren("Memory [Host]");
	RemoveChildren("Camera");
}

void QStatisticsWidget::OnPreRenderFrame(void)
{
	if (!Scene())
		return;
}

void QStatisticsWidget::OnPostRenderFrame(void)
{
	if (!Scene())
		return;
}

void QStatisticsWidget::OnStatisticChanged(const QString& Group, const QString& Name, const QString& Value, const QString& Unit /*= ""*/, const QString& Icon /*= ""*/)
{
	UpdateStatistic(Group, Name, Value, Unit);
}

void QStatisticsWidget::ExpandAll(const bool& Expand)
{
	QList<QTreeWidgetItem*> Items = m_Tree.findItems("*", Qt::MatchRecursive | Qt::MatchWildcard, 0);

	foreach (QTreeWidgetItem* pItem, Items)
		pItem->setExpanded(Expand);
}

QTreeWidgetItem* QStatisticsWidget::FindItem(const QString& Name)
{
	QList<QTreeWidgetItem*> Items = m_Tree.findItems(Name, Qt::MatchRecursive, 0);

	if (Items.size() <= 0)
		return NULL;
	else
		return Items[0];
}

void QStatisticsWidget::RemoveChildren(const QString& Name)
{
	QTreeWidgetItem* pItem = FindItem(Name);

	if (pItem)
		qDeleteAll(pItem->takeChildren());
}
