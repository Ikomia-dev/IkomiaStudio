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

#include "CWorkflowInputTypeDlg.h"
#include "Model/Project/CProjectDataProxyModel.h"

CWorkflowInputTypeDlg::CWorkflowInputTypeDlg(QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Select source of input data"), parent, DEFAULT|EFFECT_ENABLED, f)
{
    resize(QSize(300, 350));
    initLayout();
    initConnections();
}

void CWorkflowInputTypeDlg::setModel(CProjectDataProxyModel *pModel)
{
    m_pModel = pModel;
    m_pTreeView->setModel(pModel);
    m_pTreeView->expandAll();
}

CWorkflowInput CWorkflowInputTypeDlg::getInput() const
{
    return m_input;
}

int CWorkflowInputTypeDlg::exec()
{
    return QDialog::exec();
}

void CWorkflowInputTypeDlg::onInputTypeSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    TreeItemType inputType = static_cast<TreeItemType>(current->data(Qt::UserRole).toInt());
    m_input.setType(inputType);
    m_input.setMode(WorkflowInputMode::FIXED_DATA);

    switch(inputType)
    {
        case TreeItemType::IMAGE:
        case TreeItemType::VIDEO:
        case TreeItemType::FOLDER:
            showProjectView();
            break;
        case TreeItemType::DATASET:
            showInputOptions();
            break;
        default: break;
    }
}

void CWorkflowInputTypeDlg::onBack()
{
    int index = m_pStackedWidget->currentIndex();
    switch(index)
    {
        case 0:
            break;
        case 1:
            showInputTypes();
            break;
        case 2:
            if(m_input.getType() == TreeItemType::DATASET)
                showInputOptions();
            else
                showInputTypes();
            break;
    }
}

void CWorkflowInputTypeDlg::onValidate()
{
    int pageIndex = m_pStackedWidget->currentIndex();
    if(pageIndex == 1)
    {
        validateFilters();
        showProjectView();
    }
    else
        validateProjectItems();
}

void CWorkflowInputTypeDlg::initLayout()
{
    initInputTypesWidget();
    initInputOptionWidget();
    initProjectViewWidget();

    m_pStackedWidget = new QStackedWidget;
    m_pStackedWidget->addWidget(m_pInputTypesWidget);
    m_pStackedWidget->addWidget(m_pInputOptionWidget);
    m_pStackedWidget->addWidget(m_pProjectViewWidget);

    m_pBackBtn = new QPushButton(QIcon(":/Images/back.png"), "");
    m_pBackBtn->setToolTip(tr("Back"));
    m_pBackBtn->setVisible(false);
    connect(m_pBackBtn, &QPushButton::clicked, this, &CWorkflowInputTypeDlg::onBack);

    m_pNextBtn = new QPushButton(tr("Next"));
    m_pNextBtn->setVisible(false);
    connect(m_pNextBtn, &QPushButton::clicked, this, &CWorkflowInputTypeDlg::onValidate);

    m_pCancelBtn = new QPushButton(tr("Cancel"));
    connect(m_pCancelBtn, &QPushButton::clicked, this, &CWorkflowInputTypeDlg::reject);

    QHBoxLayout* pBtnLayout = new QHBoxLayout;
    pBtnLayout->addWidget(m_pBackBtn);
    pBtnLayout->addWidget(m_pNextBtn);
    pBtnLayout->addWidget(m_pCancelBtn);

    auto* pLayout = getContentLayout();
    pLayout->addWidget(m_pStackedWidget);
    pLayout->addLayout(pBtnLayout);
}

void CWorkflowInputTypeDlg::initInputTypesWidget()
{
    m_pInputTypesWidget = new QListWidget;
    m_pInputTypesWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pInputTypesWidget->setViewMode(QListWidget::IconMode);
    m_pInputTypesWidget->setFlow(QListWidget::LeftToRight);
    m_pInputTypesWidget->setResizeMode(QListWidget::Adjust);
    m_pInputTypesWidget->setGridSize(QSize(80,80));
    m_pInputTypesWidget->setSpacing(10);

    auto pItemImage = new QListWidgetItem(QIcon(":/Images/images.png"), tr("Image(s)"), m_pInputTypesWidget);
    pItemImage->setData(Qt::UserRole, static_cast<int>(TreeItemType::IMAGE));

    auto pItemVideo = new QListWidgetItem(QIcon(":/Images/video.png"), tr("Video(s)"), m_pInputTypesWidget);
    pItemVideo->setData(Qt::UserRole, static_cast<int>(TreeItemType::VIDEO));

    auto pItemDataset = new QListWidgetItem(QIcon(":/Images/dataset.png"), tr("Dataset(s)"), m_pInputTypesWidget);
    pItemDataset->setData(Qt::UserRole, static_cast<int>(TreeItemType::DATASET));

    auto pItemFolder = new QListWidgetItem(QIcon(":/Images/folder.png"), tr("Folder(s)"), m_pInputTypesWidget);
    pItemFolder->setData(Qt::UserRole, static_cast<int>(TreeItemType::FOLDER));
}

void CWorkflowInputTypeDlg::initInputOptionWidget()
{
    auto pLabelInputOption = new QLabel(tr("Select the type of data you want to process"));
    m_pCheckSimpleDataset = new QCheckBox(tr("Simple dataset"));
    m_pCheckSimpleDataset->setChecked(true);
    m_pCheckVolumeDataset = new QCheckBox(tr("Volume dataset"));
    m_pCheckVolumeDataset->setChecked(true);
    m_pCheckPositionDataset = new QCheckBox(tr("Position dataset"));
    m_pCheckPositionDataset->setChecked(true);
    m_pCheckTimeDataset = new QCheckBox(tr("Time dataset"));
    m_pCheckTimeDataset->setChecked(true);

    auto pVLayout = new QVBoxLayout;
    pVLayout->addWidget(pLabelInputOption);
    pVLayout->addWidget(m_pCheckSimpleDataset);
    pVLayout->addWidget(m_pCheckVolumeDataset);
    pVLayout->addWidget(m_pCheckPositionDataset);
    pVLayout->addWidget(m_pCheckTimeDataset);
    pVLayout->addStretch();

    m_pInputOptionWidget = new QWidget;
    m_pInputOptionWidget->setLayout(pVLayout);
}

void CWorkflowInputTypeDlg::initProjectViewWidget()
{
    m_pTreeView = new QTreeView;
    m_pTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_pTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pTreeView->setHeaderHidden(true);

    QVBoxLayout* pWidgetLayout = new QVBoxLayout;
    pWidgetLayout->addWidget(m_pTreeView);

    m_pProjectViewWidget = new QWidget;
    m_pProjectViewWidget->setLayout(pWidgetLayout);
}

void CWorkflowInputTypeDlg::initConnections()
{
    connect(m_pInputTypesWidget, &QListWidget::currentItemChanged, this, &CWorkflowInputTypeDlg::onInputTypeSelected);

    connect(m_pTreeView, &QTreeView::doubleClicked, this, &CWorkflowInputTypeDlg::onValidate);
}

void CWorkflowInputTypeDlg::showInputTypes()
{
    m_pBackBtn->setVisible(false);
    m_pNextBtn->setVisible(true);
    m_pStackedWidget->setCurrentIndex(0);
}

void CWorkflowInputTypeDlg::showInputOptions()
{
    m_pBackBtn->setVisible(true);
    m_pNextBtn->setVisible(true);
    m_pStackedWidget->setCurrentIndex(1);
}

void CWorkflowInputTypeDlg::showProjectView()
{
    emit doQueryProjectDataProxyModel(m_input.getType(), m_input.getDataFilters());
    m_pBackBtn->setVisible(true);
    m_pNextBtn->setText("OK");
    m_pNextBtn->setVisible(true);
    m_pStackedWidget->setCurrentIndex(2);
}

void CWorkflowInputTypeDlg::validateFilters()
{
    if(m_pCheckSimpleDataset->isChecked())
        m_input.addDataFilters(DataDimension::IMAGE);

    if(m_pCheckVolumeDataset->isChecked())
        m_input.addDataFilters(DataDimension::VOLUME);

    if(m_pCheckVolumeDataset->isChecked())
        m_input.addDataFilters(DataDimension::POSITION);

    if(m_pCheckTimeDataset->isChecked())
        m_input.addDataFilters(DataDimension::TIME);
}

void CWorkflowInputTypeDlg::validateProjectItems()
{
    if(m_pModel == nullptr)
        QDialog::reject();
    else
    {
        auto selectedItems = m_pTreeView->selectionModel()->selectedIndexes();
        for(int i=0; i<selectedItems.size(); ++i)
            m_input.appendModelIndex(m_pModel->mapToSource(selectedItems[i]));

        QDialog::accept();
    }
}
