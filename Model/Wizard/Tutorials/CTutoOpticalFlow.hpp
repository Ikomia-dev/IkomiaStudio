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

#ifndef CTUTOOPTICALFLOW_HPP
#define CTUTOOPTICALFLOW_HPP

#include "Model/Wizard/CWizardScenario.h"

class CTutoOpticalFlow : public CWizardScenario
{
        Q_OBJECT
    public:
        CTutoOpticalFlow(CMainModel* pModel, CMainView* pView) : CWizardScenario(pModel, pView)
        {
            makeScenario();
        }

    public slots:
        void onCheckImgprocFolder(const QModelIndex& index)
        {
            auto pModel = static_cast<const CProcessProxyModel*>(index.model());
            auto srcIndex = pModel->mapToSource(index);
            auto pItem = static_cast<CProcessModel::TreeItem*>(srcIndex.internalPointer());
            if(pItem->getTypeId() == TreeItemType::FOLDER && pItem->getName() == "Image processing")
                emit doNext();
        }
        void onCheckBilateralFilter(const QModelIndex& index)
        {
            auto pModel = static_cast<const CProcessTableProxyModel*>(index.model());
            auto srcIndex = pModel->mapToSource(index);
            auto pTable = static_cast<const QSqlTableModel*>(srcIndex.model());
            // Get name from table process (id, name, description, folder_id)
            QString name = pTable->record(index.row()).value(1).toString();
            if(name == "Bilateral Filter")
                emit doNext();
        }

    protected:
        virtual void makeScenario()
        {
            addStep(m_pView->getBtn(CMainView::BTN_OPEN_IMG),
                    "Open your first image",
                    "Please choose one or more images.",
                    "",
                    m_pModel->getProjectManager(),
                    SIGNAL(doUpdateIndex(const QModelIndex&)));
            addStep(m_pView->getBtn(CMainView::BTN_PROCESS_PANE),
                    "Open process list",
                    "Please open our process library and navigate through all our algorithms.",
                    "",
                    m_pView->getBtn(CMainView::BTN_PROCESS_PANE),
                    SIGNAL(clicked(bool)));
            addStep(m_pView->getProcessPane()->getTreeView(),
                    "Click on Image processing folder.",
                    "Please select this folder which contains some useful algorithms.",
                    "QTreeView{border: 2px solid green; border-radius: 5px}",
                    m_pView->getProcessPane()->getTreeView(),
                    SIGNAL(clicked(QModelIndex)),
                    SLOT(onCheckImgprocFolder(QModelIndex)));
            addStep(m_pView->getProcessPane()->getListPopup()->getListView(),
                    "Choose the Bilateral filter.",
                    "This process smoothes your image with an edge-preserving filter.",
                    "QListView{border: 2px solid green; border-radius: 5px}",
                    m_pView->getProcessPane()->getListPopup()->getListView(),
                    SIGNAL(clicked(QModelIndex)),
                    SLOT(onCheckBilateralFilter(QModelIndex)));
            addStep(m_pView->getProcessPane()->getProcessParameters(),
                    "Click on the Apply button",
                    "Click and watch the result on the right.",
                    "QPushButton{border: 2px solid green; border-radius: 5px}",
                    m_pView->getProcessPane(),
                    SIGNAL(doApplyProcess(std::string,std::shared_ptr<CProtocolTaskParam>)));
        }
};

class CTutoOpticalFlowFactory : public CWizardScenarioFactory
{
    public:

        CTutoOpticalFlowFactory()
        {
            m_info.m_name = "Optical flow";
            m_info.m_description = "This tutorial explains how to use the process library to apply an optical flow algorithm on a video.";
            m_info.m_authors = "Guillaume Demarcq";
            m_info.m_keywords = "video, optical, flow, process, begin, image, start, howto";
            m_info.m_iconPath = "";
        }

        virtual CWizardScenario* create(CMainModel* pModel, CMainView* pView) override
        {
            return new CTutoOpticalFlow(pModel, pView);
        }
};

#endif // CTUTOOPTICALFLOW_HPP
