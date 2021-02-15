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

#include "CGraphicsLayerChoiceDlg.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTreeView>
#include "Model/Project/CProjectGraphicsProxyModel.h"

CGraphicsLayerChoiceDlg::CGraphicsLayerChoiceDlg(QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Select graphics layer"), parent, DEFAULT|EFFECT_ENABLED, f)
{
    resize(QSize(300, 350));
    initLayout();
}

void CGraphicsLayerChoiceDlg::setModel(CProjectGraphicsProxyModel *pModel)
{
    m_pModel = pModel;
    m_pTreeView->setModel(pModel);
    m_pTreeView->expandAll();
    initConnections();
}

QModelIndex CGraphicsLayerChoiceDlg::getSelectedLayer() const
{
    return m_selectedLayerIndex;
}

int CGraphicsLayerChoiceDlg::exec()
{
    m_selectedLayerIndex = QModelIndex();
    m_pBtnAccept->setText(tr("Clear graphics input"));

    auto pSelectionModel = m_pTreeView->selectionModel();
    if(pSelectionModel)
        pSelectionModel->clear();

    return QDialog::exec();
}

void CGraphicsLayerChoiceDlg::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);

    if(selected.indexes().size() == 0)
        m_pBtnAccept->setText(tr("Clear graphics input"));
    else
        m_pBtnAccept->setText(tr("Set graphics input"));
}

void CGraphicsLayerChoiceDlg::accept()
{
    if(m_pModel == nullptr)
        QDialog::reject();
    else
    {
        auto selectedLayers = m_pTreeView->selectionModel()->selectedIndexes();
        if(selectedLayers.size() == 0)
            m_selectedLayerIndex = QModelIndex();
        else
            m_selectedLayerIndex = m_pModel->mapToSource(selectedLayers.first());

        QDialog::accept();
    }
}

void CGraphicsLayerChoiceDlg::initLayout()
{
    m_pTreeView = new QTreeView;
    m_pTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    //m_pTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pTreeView->setHeaderHidden(true);

    m_pBtnAccept = new QPushButton(tr("Clear graphics input"));
    connect(m_pBtnAccept, &QPushButton::clicked, this, &CGraphicsLayerChoiceDlg::accept);

    QPushButton* pBtnCancel = new QPushButton(tr("Cancel"));
    connect(pBtnCancel, &QPushButton::clicked, this, &CGraphicsLayerChoiceDlg::reject);

    QHBoxLayout* pBtnLayout = new QHBoxLayout;
    pBtnLayout->addWidget(m_pBtnAccept);
    pBtnLayout->addWidget(pBtnCancel);

    auto pLayout = getContentLayout();
    pLayout->addWidget(m_pTreeView);
    pLayout->addLayout(pBtnLayout);
}

void CGraphicsLayerChoiceDlg::initConnections()
{
    connect(m_pTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CGraphicsLayerChoiceDlg::onSelectionChanged);
}
