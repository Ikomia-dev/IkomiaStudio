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

#include "CCppNewPluginWidget.h"
#include <QtWidgets>
#include "PluginManagerDefine.hpp"
#include "CCppPluginMaker.h"
#include "UtilsTools.hpp"

CCppNewPluginWidget::CCppNewPluginWidget(QWidget *parent) : QWidget(parent)
{
    initLayout();
    initApiFolder();
    initConnections();
}

void CCppNewPluginWidget::onBrowseSrcFolder()
{
    m_pEditCppSrcFolder->setText(getSelectedFolder() + "/" + m_pEditCppName->text());
}

void CCppNewPluginWidget::onBrowseApiFolder()
{
    m_pEditCppApiFolder->setText(getSelectedFolder());
}

void CCppNewPluginWidget::onGeneratePlugin()
{
    try
    {
        QString pluginName = m_pEditCppName->text();
        if(pluginName.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Empty plugin name"));
            msgBox.exec();
            return;
        }

        CCppPluginMaker cppMaker;
        cppMaker.setName(pluginName);
        cppMaker.setSrcFolder(m_pEditCppSrcFolder->text());
        cppMaker.setApiFolder(m_pEditCppApiFolder->text());
        cppMaker.setProcessBaseClass(static_cast<PluginManager::ProcessBaseClass>(m_pComboCppProcessType->currentData().toInt()));
        cppMaker.setWidgetBaseClass(static_cast<PluginManager::WidgetBaseClass>(m_pComboCppWidgetType->currentData().toInt()));
        cppMaker.generate();

        emit doPluginCreated(pluginName);
    }
    catch(std::exception& e)
    {
        qCritical().noquote() << QString::fromStdString(e.what());
    }
}

void CCppNewPluginWidget::initLayout()
{
    auto pLabelName = new QLabel(tr("Plugin name"));
    m_pEditCppName = new QLineEdit;

    auto pLabelSrcFolder = new QLabel(tr("Source folder"));
    m_pEditCppSrcFolder = new QLineEdit;
    m_pBtnBrowseSrcFolder = new QPushButton("...");

    auto pLabelApiFolder = new QLabel(tr("Ikomia API folder"));
    m_pEditCppApiFolder = new QLineEdit;
    m_pBtnBrowseApiFolder = new QPushButton("...");

    auto pLabelProcessType = new QLabel(tr("Process type"));
    m_pComboCppProcessType = new QComboBox;
    m_pComboCppProcessType->addItem(tr("Generic process"), PluginManager::CWorkflow_TASK);
    m_pComboCppProcessType->addItem(tr("Image process"), PluginManager::CIMAGE_PROCESS_2D);
    m_pComboCppProcessType->addItem(tr("Interactive image process"), PluginManager::CINTERACTIVE_IMAGE_PROCESS_2D);
    m_pComboCppProcessType->addItem(tr("Video process"), PluginManager::CVIDEO_PROCESS);
    m_pComboCppProcessType->addItem(tr("Optical flow process"), PluginManager::CVIDEO_PROCESS_OF);
    m_pComboCppProcessType->setCurrentIndex(1);

    auto pLabelWidgetType = new QLabel(tr("Widget type"));
    m_pComboCppWidgetType = new QComboBox;
    m_pComboCppWidgetType->addItem(tr("Empty widget"), PluginManager::CWorkflow_TASK_WIDGET);
    m_pComboCppWidgetType->setCurrentIndex(0);

    auto pLayout = new QGridLayout;
    pLayout->addWidget(pLabelName, 0, 0);
    pLayout->addWidget(m_pEditCppName, 0, 1, 1, 2);
    pLayout->addWidget(pLabelSrcFolder, 1, 0);
    pLayout->addWidget(m_pEditCppSrcFolder, 1, 1);
    pLayout->addWidget(m_pBtnBrowseSrcFolder, 1, 2);
    pLayout->addWidget(pLabelApiFolder, 2, 0);
    pLayout->addWidget(m_pEditCppApiFolder, 2, 1);
    pLayout->addWidget(m_pBtnBrowseApiFolder, 2, 2);
    pLayout->addWidget(pLabelProcessType, 3, 0);
    pLayout->addWidget(m_pComboCppProcessType, 3, 1, 1, 2);
    pLayout->addWidget(pLabelWidgetType, 4, 0);
    pLayout->addWidget(m_pComboCppWidgetType, 4, 1, 1, 2);

    m_pBtnGenerate = new QPushButton(tr("Generate"));
    m_pBtnGenerate->setMinimumWidth(200);

    auto pMainLayout = new QHBoxLayout;
    pMainLayout->addLayout(pLayout);
    pMainLayout->addWidget(m_pBtnGenerate);
    pMainLayout->addStretch(1);
    pMainLayout->setStretch(0, 2);
    pMainLayout->setStretch(1, 1);
    pMainLayout->setStretch(2, 2);

    setLayout(pMainLayout);
}

void CCppNewPluginWidget::initApiFolder()
{
#ifdef QT_DEBUG
    #ifdef _WIN32
        m_pEditCppApiFolder->setText("c:/Developpement/Ikomia/Install/Api");
    #else
        m_pEditCppApiFolder->setText(QDir::homePath() + "/Developpement/Ikomia/Install/Api");
    #endif
#else
    m_pEditCppApiFolder->setText(Utils::IkomiaApp::getQAppFolder() + "/Api");
#endif
}

void CCppNewPluginWidget::initConnections()
{
    connect(m_pBtnBrowseSrcFolder, &QPushButton::clicked, this, &CCppNewPluginWidget::onBrowseSrcFolder);
    connect(m_pBtnBrowseApiFolder, &QPushButton::clicked, this, &CCppNewPluginWidget::onBrowseApiFolder);
    connect(m_pBtnGenerate, &QPushButton::clicked, this, &CCppNewPluginWidget::onGeneratePlugin);
}

QString CCppNewPluginWidget::getSelectedFolder()
{
    QString filePath("");
    QFileDialog fileDlg(this);
    fileDlg.setFileMode(QFileDialog::DirectoryOnly);
    fileDlg.setViewMode(QFileDialog::Detail);

    if(fileDlg.exec())
    {
        QStringList pathList = fileDlg.selectedFiles();
        filePath = pathList.first();
    }
    return filePath;
}
