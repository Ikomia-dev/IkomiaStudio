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

#include "CWizardStepPage.h"
#include "CWizardStepListView.h"
#include "Model/Wizard/CWizardStepModel.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

CWizardStepPage::CWizardStepPage(QWidget* parent) : QFrame(parent)
{
    init();
}

void CWizardStepPage::setTitle(const QString& name)
{
    m_pTitle->setText(name);
}

void CWizardStepPage::setTotalSteps(int totalSteps)
{
    m_totalSteps = totalSteps;
}

void CWizardStepPage::setCurrentStep(int step)
{
    m_pStep->setText(QString("Step %1/%2").arg(step).arg(m_totalSteps));
}

void CWizardStepPage::setDescription(const QString& description)
{
    m_pDescription->setText(description);
}

void CWizardStepPage::setListViewModel(CWizardStepModel* pModel)
{
    m_pListView->setModel(pModel);
}

void CWizardStepPage::init()
{
    initLayout();
}

void CWizardStepPage::initLayout()
{
    createGlobalStepView();
    createDetailStepView();

    QPushButton* pCancelBtn = new QPushButton(tr("Cancel"));
    connect(pCancelBtn, &QPushButton::clicked, [&]{ emit doCancelTuto(); });

    QVBoxLayout* pVbox = new QVBoxLayout;
    pVbox->addWidget(m_pGlobalStepFrame);
    pVbox->addWidget(m_pDetailStepFrame);
    pVbox->addWidget(pCancelBtn);

    setLayout(pVbox);
}

void CWizardStepPage::createGlobalStepView()
{
    m_pListView = new CWizardStepListView;

    QLabel* pLabelGraph = new QLabel(tr("Step by step summary"));
    pLabelGraph->setStyleSheet("background: transparent; border: transparent;");
    QHBoxLayout* pHBoxTitleGraph = new QHBoxLayout;
    pHBoxTitleGraph->addWidget(pLabelGraph);
    QFrame* pFrameTitleGraph = new QFrame;
    pFrameTitleGraph->setProperty("class", "CHeaderDialogStyle");
    pFrameTitleGraph->setLayout(pHBoxTitleGraph);
    QVBoxLayout* pVBoxGraph = new QVBoxLayout;
    pVBoxGraph->setContentsMargins(0,0,0,0);
    pVBoxGraph->setSpacing(0);
    pVBoxGraph->addWidget(pFrameTitleGraph);
    pVBoxGraph->addWidget(m_pListView);
    m_pGlobalStepFrame = new QFrame;
    m_pGlobalStepFrame->setLayout(pVBoxGraph);
    m_pGlobalStepFrame->setProperty("class", "CDialogStyle");
}

void CWizardStepPage::createDetailStepView()
{
    QLabel* pLabelDetail = new QLabel(tr("Step details"));
    pLabelDetail->setStyleSheet("background: transparent; border: transparent;");
    QHBoxLayout* pHBoxTitleDetail = new QHBoxLayout;
    pHBoxTitleDetail->addWidget(pLabelDetail);
    QFrame* pFrameTitleDetail = new QFrame;
    pFrameTitleDetail->setProperty("class", "CHeaderDialogStyle");
    pFrameTitleDetail->setLayout(pHBoxTitleDetail);

    m_pTitle = new QLabel;
    m_pTitle->setContentsMargins(5,5,5,5);
    m_pTitle->setStyleSheet("background: rgb(70,70,70); border-bottom: 1px solid #212121; border-right: 1px solid #212121; border-radius: 0px;");
    m_pTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_pStep = new QLabel;
    m_pStep->setContentsMargins(5,5,5,5);
    m_pStep->setStyleSheet("background: rgb(70,70,70); border-bottom: 1px solid #212121; border-radius: 0px;");
    m_pStep->setAlignment(Qt::AlignRight);

    m_pDescription = new QLabel;
    m_pDescription->setProperty("class", "CLightGrayLabel");
    m_pDescription->setAlignment(Qt::AlignCenter);
    m_pDescription->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_pDescription->setWordWrap(true);

    m_pDetailStepFrame = new QFrame;
    QHBoxLayout* pHBox = new QHBoxLayout;
    pHBox->setContentsMargins(0,0,0,0);
    pHBox->setSpacing(0);
    pHBox->addWidget(m_pTitle);
    pHBox->addWidget(m_pStep);
    QVBoxLayout* pVBox = new QVBoxLayout;
    pVBox->setContentsMargins(0,0,0,0);
    pVBox->setSpacing(0);
    pVBox->addWidget(pFrameTitleDetail);
    pVBox->addLayout(pHBox);
    pVBox->addWidget(m_pDescription);

    m_pDetailStepFrame->setLayout(pVBox);
    m_pDetailStepFrame->setProperty("class", "CDialogStyle");
}
