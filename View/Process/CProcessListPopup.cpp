// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CProcessListPopup.h"
#include <QListView>
#include <QTreeView>
#include <QVBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QSqlTableModel>

CProcessListPopup::CProcessListPopup(QWidget *parent, Qt::WindowFlags f) :
    CDialog(parent, NO_TITLE_BAR, f)
{
    setSizeGripEnabled(false);
    setWindowOpacity(0.9);
    initLayout();
    initConnections();
}

void CProcessListPopup::setModel(QSortFilterProxyModel* pModel)
{
    m_pProcessList->setModel(pModel);
    m_pProcessList->setModelColumn(1);
}

void CProcessListPopup::setCurrentCategory(QString name)
{
    m_currentCategory = name;
    m_pLabel->setText(name);
}

bool CProcessListPopup::isCurrent(QString name) const
{
    return m_currentCategory == name;
}

CResizeListView*CProcessListPopup::getListView()
{
    return m_pProcessList;
}

void CProcessListPopup::resizeListView()
{
    m_pProcessList->updateGeometry();
    m_pFrame->updateGeometry();
    updateGeometry();

    QRect rec = QApplication::primaryScreen()->availableGeometry();
    QSize size = sizeHint();

    if(size.height() < rec.height())
        resize(size.width(), size.height());
    else
        resize(size.width(), rec.height());
}

void CProcessListPopup::onListViewClicked(const QModelIndex& index)
{
    assert(index.isValid());
    // Don't forget that we are using a proxy model
    auto pProxyModel = static_cast<QSortFilterProxyModel*>(m_pProcessList->model());
    auto currIndex = pProxyModel->mapToSource(index);
    // Get QSqlTableModel
    auto pTable = static_cast<const QSqlTableModel*>(currIndex.model());
    // Get name from table process (id, name, description, folder_id)
    auto name = pTable->record(index.row()).value(1).toString();

    emit doShowWidget(name);
}

void CProcessListPopup::initLayout()
{
    m_pProcessList = new CResizeListView;
    m_pProcessList->setIconSize(QSize(16, 16));
    m_pProcessList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pProcessList->setUniformItemSizes(true);

    //Label with line underneath
    m_pLabel = new QLabel(tr("Process list"));
    m_pLabel->setAlignment(Qt::AlignCenter);

    auto pLine = new QFrame;
    pLine->setLineWidth(1);
    pLine->setFrameStyle(QFrame::Plain);
    pLine->setFrameShape(QFrame::HLine);
    pLine->setStyleSheet(QString("color:%1;").arg(qApp->palette().highlight().color().name()));

    //Add to layout
    auto pLayout = getContentLayout();
    pLayout->addWidget(m_pLabel);
    pLayout->addWidget(pLine);
    pLayout->addWidget(m_pProcessList);
}

void CProcessListPopup::initConnections()
{
    connect(m_pProcessList, &CResizeListView::clicked, this, &CProcessListPopup::onListViewClicked);
}
