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

#include "CWizardPane.h"
#include "Model/Wizard/CWizardQueryModel.h"
#include "Model/Wizard/CWizardStepModel.h"
#include <QListView>
#include <QVBoxLayout>
#include <QToolBox>
#include <QStringListModel>
#include <QLineEdit>

CWizardPane::CWizardPane(QWidget *parent) : QWidget(parent)
{
    init();
}

CWizardTutoListView*CWizardPane::getTutoListView()
{
    return m_pListView;
}

CWizardStepPage* CWizardPane::getWizardStepPage()
{
    return m_pStepPage;
}

CWizardDocPage* CWizardPane::getWizardDocPage()
{
    return m_pDocPage;
}

void CWizardPane::onSetTutorialModel(CWizardQueryModel* pModel)
{
    m_pListView->setModel(pModel);
}

void CWizardPane::onSetStepModel(CWizardStepModel* pModel)
{
    m_pStepPage->setListViewModel(pModel);
}

void CWizardPane::onSetTotalSteps(int totalSteps)
{
    m_pStepPage->setTotalSteps(totalSteps);
}

void CWizardPane::onShowStepPage(const QString& stepName, int currentStep, const QString& description)
{
    m_pStepPage->setTitle(stepName);
    m_pStepPage->setCurrentStep(currentStep);
    m_pStepPage->setDescription(description);

    m_pStack->setCurrentIndex(m_stepPageIndex);
}

void CWizardPane::onShowDocPage(const QModelIndex& /*index*/)
{
    m_pStack->setCurrentIndex(m_docPageIndex);
}

void CWizardPane::onShowMainPage()
{
    m_pStack->setCurrentIndex(m_mainPageIndex);
}

void CWizardPane::onFinish()
{
    m_pStack->setCurrentIndex(m_mainPageIndex);
}

void CWizardPane::init()
{
    initLayout();
    initConnections();
}

void CWizardPane::initLayout()
{
    QToolBox* pToolbox = new QToolBox;
    QVBoxLayout* pLayout = new QVBoxLayout;

    //

    m_pStack = new QStackedWidget;

    createMainPage();
    createStepPage();
    createDocPage();

    m_mainPageIndex = m_pStack->addWidget(m_pMainPage);
    m_stepPageIndex = m_pStack->addWidget(m_pStepPage);
    m_docPageIndex = m_pStack->addWidget(m_pDocPage);
    m_pStack->setCurrentIndex(0);

    //

    pToolbox->addItem(m_pStack, tr("Tutorials"));

    pLayout->addWidget(pToolbox);
    setLayout(pLayout);
}

void CWizardPane::initConnections()
{
    connect(m_pListView, &CWizardTutoListView::doShowTutoInfo, this, &CWizardPane::onShowDocPage);
    connect(m_pDocPage, &CWizardDocPage::doBack, this, &CWizardPane::onShowMainPage);
}

void CWizardPane::createMainPage()
{
    m_pMainPage = new QFrame;
    m_pListView = new CWizardTutoListView;

    QLineEdit* pSearchEdit = new QLineEdit;
    pSearchEdit->setPlaceholderText(tr("<Search tutorials by keywords>"));

    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(pSearchEdit);
    pLayout->addWidget(m_pListView);

    m_pMainPage->setLayout(pLayout);
}

void CWizardPane::createDocPage()
{
    m_pDocPage = new CWizardDocPage(CWizardDocPage::BACK);
}

void CWizardPane::createStepPage()
{
    m_pStepPage = new CWizardStepPage;
}
