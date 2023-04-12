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

#include "CPythonNewPluginDlg.h"
#include <QtWidgets>
#include "PluginManagerDefine.hpp"
#include "CPythonPluginMaker.h"

CPythonNewPluginDlg::CPythonNewPluginDlg(QWidget *parent, Qt::WindowFlags f) :
    CDialog(tr("New Python plugin"), parent, DEFAULT|EFFECT_ENABLED, f)
{
    initLayout();
    initConnections();
}

QString CPythonNewPluginDlg::getName() const
{
    return m_name;
}

void CPythonNewPluginDlg::onGeneratePlugin()
{
    try
    {
        m_name = m_pEditPyName->text();
        if(m_name.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Empty plugin name"));
            msgBox.exec();
            return;
        }

        CPythonPluginMaker pythonMaker;
        pythonMaker.setName(m_name);
        pythonMaker.setProcessBaseClass(static_cast<PluginManager::ProcessBaseClass>(m_pComboPyProcessType->currentData().toInt()));
        pythonMaker.setWidgetBaseClass(static_cast<PluginManager::WidgetBaseClass>(m_pComboPyWidgetType->currentData().toInt()));
        pythonMaker.setQtBinding(static_cast<CPythonPluginMaker::QtBinding>(m_pComboPyQt->currentData().toInt()));
        pythonMaker.generate();
        emit accept();
    }
    catch(std::exception& e)
    {
        qCritical().noquote() << QString::fromStdString(e.what());
    }
}

void CPythonNewPluginDlg::initLayout()
{
    auto pLabelName = new QLabel(tr("Algorithm name"));
    m_pEditPyName = new QLineEdit;

    auto pLabelProcessType = new QLabel(tr("Algorithm type"));
    m_pComboPyProcessType = new QComboBox;
    m_pComboPyProcessType->addItem(tr("Generic algorithm"), PluginManager::CWORKFLOW_TASK);
    m_pComboPyProcessType->addItem(tr("Image algorithm"), PluginManager::C2DIMAGE_TASK);
    m_pComboPyProcessType->addItem(tr("Interactive image algorithm"), PluginManager::C2DIMAGE_INTERACTIVE_TASK);
    m_pComboPyProcessType->addItem(tr("Video algorithm"), PluginManager::CVIDEO_TASK);
    m_pComboPyProcessType->addItem(tr("Classification algorithm"), PluginManager::CCLASSIFICATION_TASK);
    m_pComboPyProcessType->addItem(tr("Object detection algorithm"), PluginManager::COBJECT_DETECTION_TASK);
    m_pComboPyProcessType->addItem(tr("Semantic segmentation algorithm"), PluginManager::CSEMANTIC_SEGMENTATION_TASK);
    m_pComboPyProcessType->addItem(tr("Instance segmentation algorithm"), PluginManager::CINSTANCE_SEGMENTATION_TASK);
    m_pComboPyProcessType->addItem(tr("Keypoints detection algorithm"), PluginManager::CKEYPOINT_DETECTION_TASK);
    m_pComboPyProcessType->addItem(tr("Optical flow algorithm"), PluginManager::CVIDEO_OF_TASK);
    m_pComboPyProcessType->setCurrentIndex(1);

    auto pLabelWidgetType = new QLabel(tr("Widget type"));
    m_pComboPyWidgetType = new QComboBox;
    m_pComboPyWidgetType->addItem(tr("Empty widget"), PluginManager::CWORKFLOW_TASK_WIDGET);
    m_pComboPyWidgetType->setCurrentIndex(0);

    auto pLabelFramework = new QLabel(tr("GUI framework"));
    m_pComboPyQt = new QComboBox;
    m_pComboPyQt->addItem("PyQt", CPythonPluginMaker::PYQT);
    m_pComboPyQt->addItem("PySide", CPythonPluginMaker::PYSIDE);
    m_pComboPyQt->setCurrentIndex(0);

    auto pCentralLayout = new QGridLayout;
    pCentralLayout->addWidget(pLabelName, 0, 0);
    pCentralLayout->addWidget(m_pEditPyName, 0, 1);
    pCentralLayout->addWidget(pLabelProcessType, 1, 0);
    pCentralLayout->addWidget(m_pComboPyProcessType, 1, 1);
    pCentralLayout->addWidget(pLabelWidgetType, 2, 0);
    pCentralLayout->addWidget(m_pComboPyWidgetType, 2, 1);
    pCentralLayout->addWidget(pLabelFramework, 3, 0);
    pCentralLayout->addWidget(m_pComboPyQt, 3, 1);

    m_pBtnGenerate = new QPushButton(tr("Generate"));
    m_pBtnGenerate->setDefault(true);

    m_pBtnCancel = new QPushButton(tr("Cancel"));

    QHBoxLayout* pBtnLayout = new QHBoxLayout;
    pBtnLayout->addWidget(m_pBtnGenerate);
    pBtnLayout->addWidget(m_pBtnCancel);

    QVBoxLayout* pMainLayout = getContentLayout();
    pMainLayout->addLayout(pCentralLayout);
    pMainLayout->addLayout(pBtnLayout);
}

void CPythonNewPluginDlg::initConnections()
{
    connect(m_pBtnGenerate, &QPushButton::clicked, this, &CPythonNewPluginDlg::onGeneratePlugin);
    connect(m_pBtnCancel, &QPushButton::clicked, this, &CPythonNewPluginDlg::reject);
}
