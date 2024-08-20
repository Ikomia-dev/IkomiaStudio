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

#include "CPythonPluginManagerWidget.h"
#include <QtWidgets>
#include "Model/Plugin/CPluginPythonModel.h"
#include "Model/Plugin/CPluginPythonDependencyModel.h"
#include "CPythonNewPluginDlg.h"

CPythonPluginManagerWidget::CPythonPluginManagerWidget(QWidget *parent) : QWidget(parent)
{
    initLayout();
    initConnections();
}

void CPythonPluginManagerWidget::setModel(CPluginPythonModel *pModel)
{
    m_pPluginsView->setModel(pModel);
    m_pPluginsView->horizontalHeader()->setMinimumSectionSize(150);
    m_pPluginsView->horizontalHeader()->setMaximumSectionSize(650);
    m_pPluginsView->resizeColumnsToContents();
    adjustTableSize(m_pPluginsView);
    connect(m_pPluginsView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CPythonPluginManagerWidget::onPluginSelectionChanged);
}

void CPythonPluginManagerWidget::setDependencyModel(CPluginPythonDependencyModel *pModel)
{
    m_pDependenciesView->setModel(pModel);
    m_pDependenciesView->horizontalHeader()->setMinimumSectionSize(150);
    m_pDependenciesView->horizontalHeader()->setMaximumSectionSize(650);
    m_pDependenciesView->resizeColumnsToContents();
    adjustTableSize(m_pDependenciesView);
}

void CPythonPluginManagerWidget::onNewPlugin()
{
    CPythonNewPluginDlg newPluginDlg(this);
    if(newPluginDlg.exec() == QDialog::Accepted)
    {
        emit doPluginCreated(newPluginDlg.getName());
        emit doReloadAll();
    }
}

void CPythonPluginManagerWidget::onReloadPlugin()
{
    auto pModel = static_cast<CPluginPythonModel*>(m_pPluginsView->model());
    QItemSelectionModel* pSelectionModel = m_pPluginsView->selectionModel();
    QModelIndexList indexes = pSelectionModel->selectedRows();

    for(int i=0; i<indexes.size(); ++i)
    {
        QStandardItem* pItem = pModel->itemFromIndex(indexes[i]);
        emit doReload(pItem->data(Qt::DisplayRole).toString());
    }
}

void CPythonPluginManagerWidget::onEditPlugin()
{
    auto pModel = static_cast<CPluginPythonModel*>(m_pPluginsView->model());
    QItemSelectionModel* pSelectionModel = m_pPluginsView->selectionModel();
    QModelIndexList indexes = pSelectionModel->selectedRows();

    for(int i=0; i<indexes.size(); ++i)
    {
        QStandardItem* pItem = pModel->itemFromIndex(indexes[i]);
        emit doEdit(pItem->data(Qt::DisplayRole).toString());
    }
}

void CPythonPluginManagerWidget::onShowLocation()
{
    auto pModel = static_cast<CPluginPythonModel*>(m_pPluginsView->model());
    QItemSelectionModel* pSelectionModel = m_pPluginsView->selectionModel();
    QModelIndexList indexes = pSelectionModel->selectedRows();

    if(indexes.size() > 0)
    {
        QStandardItem* pItem = pModel->itemFromIndex(indexes.first());
        emit doShowLocation(pItem->data(Qt::DisplayRole).toString());
    }
}

void CPythonPluginManagerWidget::onPluginSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    if(selected.indexes().size() == 0)
    {
        QApplication::restoreOverrideCursor();
        return;
    }

    auto pModel = static_cast<CPluginPythonModel*>(m_pPluginsView->model());
    QStandardItem* pItem = pModel->itemFromIndex(selected.indexes().first());
    emit doGetPluginDependencyModel(pItem->data(Qt::DisplayRole).toString());
    QApplication::restoreOverrideCursor();
}

void CPythonPluginManagerWidget::onInstallDependency()
{
    auto pDependencyModel = static_cast<CPluginPythonModel*>(m_pDependenciesView->model());
    QItemSelectionModel* pSelectionModel = m_pDependenciesView->selectionModel();

    if(pSelectionModel == nullptr)
        return;

    QModelIndexList indexes = pSelectionModel->selectedRows();
    for(int i=0; i<indexes.size(); ++i)
    {
        QStandardItem* pItem = pDependencyModel->itemFromIndex(indexes[i]);
        emit doInstall(pItem->data(Qt::DisplayRole).toString());
    }
}

void CPythonPluginManagerWidget::onUpdateDependency()
{
    auto pModel = static_cast<CPluginPythonModel*>(m_pDependenciesView->model());
    QItemSelectionModel* pSelectionModel = m_pDependenciesView->selectionModel();

    if(pSelectionModel == nullptr)
        return;

    QModelIndexList indexes = pSelectionModel->selectedRows();
    for(int i=0; i<indexes.size(); ++i)
    {
        QStandardItem* pItem = pModel->itemFromIndex(indexes[i]);
        emit doUpdate(pItem->data(Qt::DisplayRole).toString());
    }
}

void CPythonPluginManagerWidget::onInstallRequirements()
{
    auto pModel = static_cast<CPluginPythonModel*>(m_pPluginsView->model());
    QItemSelectionModel* pSelectionModel = m_pPluginsView->selectionModel();
    QModelIndexList indexes = pSelectionModel->selectedRows();

    if(indexes.size() > 0)
    {
        QStandardItem* pItem = pModel->itemFromIndex(indexes.first());
        emit doInstallRequirements(pItem->data(Qt::DisplayRole).toString());
    }
}

void CPythonPluginManagerWidget::initLayout()
{
    m_pPluginsView = new QTableView();
    m_pPluginsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pPluginsView->setWordWrap(true);

    m_pNewBtn = createButton(QIcon(":/Images/new.png"), tr("Create new plugin..."));
    m_pReloadBtn = createButton(QIcon(":/Images/update.png"), tr("Reload selected plugin"));
    m_pReloadAllBtn = createButton(QIcon(":/Images/update-all.png"), tr("Reload all plugins"));
    m_pInstallBtn = createButton(QIcon(":/Images/install.png"), tr("Install selected plugin requirements"));
    m_pEditBtn = createButton(QIcon(":/Images/edit.png"), tr("Edit plugin"));
    m_pShowLocationBtn = createButton(QIcon(":/Images/show-location.png"), tr("Show location"));

    auto pPluginBtnLayout = new QVBoxLayout;
    pPluginBtnLayout->addWidget(m_pNewBtn);
    pPluginBtnLayout->addWidget(m_pReloadBtn);
    pPluginBtnLayout->addWidget(m_pReloadAllBtn);
    pPluginBtnLayout->addWidget(m_pInstallBtn);
    pPluginBtnLayout->addWidget(m_pEditBtn);
    pPluginBtnLayout->addWidget(m_pShowLocationBtn);
    pPluginBtnLayout->addStretch(1);

    m_pDependenciesView = new QTableView;
    m_pDependenciesView->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_pInstallPkgBtn = createButton(QIcon(":/Images/install.png"), tr("Install selected package"));
    m_pUpdatePkgBtn = createButton(QIcon(":/Images/update.png"), tr("Update selected package"));

    auto pDependencyBtnLayout = new QVBoxLayout;
    pDependencyBtnLayout->addWidget(m_pInstallPkgBtn);
    pDependencyBtnLayout->addWidget(m_pUpdatePkgBtn);
    pDependencyBtnLayout->addStretch(1);

    auto pMainLayout = new QHBoxLayout;
    pMainLayout->addWidget(m_pPluginsView);
    pMainLayout->addLayout(pPluginBtnLayout);
    pMainLayout->addSpacing(50);
    pMainLayout->addWidget(m_pDependenciesView);
    pMainLayout->addLayout(pDependencyBtnLayout);

    setLayout(pMainLayout);
}

void CPythonPluginManagerWidget::initConnections()
{
    connect(m_pNewBtn, &QPushButton::clicked, this, &CPythonPluginManagerWidget::onNewPlugin);
    connect(m_pReloadBtn, &QPushButton::clicked, this, &CPythonPluginManagerWidget::onReloadPlugin);
    connect(m_pReloadAllBtn, &QPushButton::clicked, [&]{ emit doReloadAll(); });
    connect(m_pInstallBtn, &QPushButton::clicked, this, &CPythonPluginManagerWidget::onInstallRequirements);
    connect(m_pEditBtn, &QPushButton::clicked, this, &CPythonPluginManagerWidget::onEditPlugin);
    connect(m_pShowLocationBtn, &QPushButton::clicked, this, &CPythonPluginManagerWidget::onShowLocation);
    connect(m_pInstallPkgBtn, &QPushButton::clicked, this, &CPythonPluginManagerWidget::onInstallDependency);
    connect(m_pUpdatePkgBtn, &QPushButton::clicked, this, &CPythonPluginManagerWidget::onUpdateDependency);
}

QPushButton *CPythonPluginManagerWidget::createButton(const QIcon& icon, const QString& tooltip)
{
    auto pal = qApp->palette();
    auto color = pal.highlight().color();
    auto pBtn = new QPushButton;

    pBtn->setIcon(icon);
    pBtn->setFlat(true);
    pBtn->setFixedSize(QSize(24, 24));
    pBtn->setIconSize(QSize(20, 20));
    pBtn->setToolTip(tooltip);
    pBtn->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}").arg(color.name()));
    return pBtn;
}

void CPythonPluginManagerWidget::adjustTableSize(QTableView* pTableView)
{
    int headerLength = pTableView->horizontalHeader()->length();
    headerLength += pTableView->verticalHeader()->width();
    headerLength += pTableView->verticalScrollBar()->sizeHint().width();
    headerLength += 2 * pTableView->frameWidth();
    pTableView->setMinimumWidth(headerLength);
}


