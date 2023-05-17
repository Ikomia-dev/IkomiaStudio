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

#ifndef CTUTOSTARTINHELPER_HPP
#define CTUTOSTARTINHELPER_HPP

#include "Model/Wizard/CWizardScenario.h"
#include "View/Store/CStoreDlg.h"
#include "View/Common/CBubbleTip.h"

class CTutoStartingHelper : public CWizardScenario
{
        Q_OBJECT
    public:
        CTutoStartingHelper(CMainModel* pModel, CMainView* pView) : CWizardScenario(pModel, pView)
        {
            makeScenario();
        }

    public slots:

    protected:
        virtual void makeScenario()
        {
            // Show authentication
            addStep(m_pView->getBtn(CMainView::BTN_OPEN_LOGIN),
                    tr("Click here to sign into your account."),
                    "",
                    "",
                    m_pView->getBtn(CMainView::BTN_OPEN_LOGIN),
                    SIGNAL(clicked(bool)),
                    nullptr,
                    CBubbleTip::Close | CBubbleTip::Check);

            // Open Project
            addStep(m_pView->getBtn(CMainView::BTN_OPEN_PROJECT),
                    tr("Open your project."),
                    "",
                    "",
                    m_pView->getBtn(CMainView::BTN_OPEN_PROJECT),
                    SIGNAL(clicked(bool)),
                    nullptr,
                    CBubbleTip::Close | CBubbleTip::Check | CBubbleTip::Next);

            // Open Image
            addStep(m_pView->getBtn(CMainView::BTN_OPEN_IMG),
                    tr("Open your image(s)."),
                    "",
                    "",
                    m_pView->getBtn(CMainView::BTN_OPEN_IMG),
                    SIGNAL(clicked(bool)),
                    nullptr,
                    CBubbleTip::Close | CBubbleTip::Check | CBubbleTip::Next);

            // Open Video
            addStep(m_pView->getBtn(CMainView::BTN_OPEN_VIDEO),
                    tr("Open your video(s)."),
                    "",
                    "",
                    m_pView->getBtn(CMainView::BTN_OPEN_VIDEO),
                    SIGNAL(clicked(bool)),
                    nullptr,
                    CBubbleTip::Close | CBubbleTip::Check | CBubbleTip::Next);

            // Open Stream
            addStep(m_pView->getBtn(CMainView::BTN_OPEN_STREAM),
                    tr("Open your streaming device."),
                    "",
                    "",
                    m_pView->getBtn(CMainView::BTN_OPEN_STREAM),
                    SIGNAL(clicked(bool)),
                    nullptr,
                    CBubbleTip::Close | CBubbleTip::Check | CBubbleTip::Next);

            // Project Pane
            addStep(m_pView->getBtn(CMainView::BTN_PROJECT_PANE),
                    tr("Browse through your projects."),
                    "",
                    "",
                    m_pView->getBtn(CMainView::BTN_PROJECT_PANE),
                    SIGNAL(clicked(bool)),
                    nullptr,
                    CBubbleTip::Close | CBubbleTip::Check | CBubbleTip::Next);

            // Process Pane
            addStep(m_pView->getBtn(CMainView::BTN_PROCESS_PANE),
                    tr("Click here to see all available algorithms."),
                    "",
                    "",
                    m_pView->getBtn(CMainView::BTN_PROCESS_PANE),
                    SIGNAL(clicked(bool)),
                    nullptr,
                    CBubbleTip::Close | CBubbleTip::Check);

            // Process List
            addStep(m_pView->getProcessPane()->getTreeView(),
                    tr("All algorithms are listed here (OpenCV, G'MIC, plugins and algorithms installed from the Ikomia HUB)."),
                    "",
                    "QTreeView{border: 2px solid green; border-radius: 5px}",
                    m_pView->getProcessPane()->getTreeView(),
                    SIGNAL(clicked(QModelIndex)),
                    nullptr,
                    CBubbleTip::Close | CBubbleTip::Check | CBubbleTip::Next);

            // Process search engine
            addStep(m_pView->getProcessPane()->getSearchBox(),
                    tr("Type your keyword (e.g. smooth) and find your favorite algorithm!"),
                    "",
                    "",
                    m_pView->getBtn(CMainView::BTN_PROCESS_PANE),
                    SIGNAL(clicked(bool)),
                    nullptr,
                    CBubbleTip::Close | CBubbleTip::Check | CBubbleTip::Next);

            // Open Store
            addStep(m_pView->getBtn(CMainView::BTN_OPEN_STORE),
                    tr("Open Ikomia HUB and leverage state of the art algorithms."),
                    "",
                    "",
                    m_pView->getStoreView(),
                    SIGNAL(doClose()),
                    nullptr,
                    CBubbleTip::Close | CBubbleTip::Check);

            // Workflow Pane
            addStep(m_pView->getBtn(CMainView::BTN_MODULE_WORKFLOW),
                    tr("Open the workflow creator. Test, chain or compare your algorithms. Enjoy Ikomia!"),
                    "",
                    "",
                    m_pView->getBtn(CMainView::BTN_MODULE_WORKFLOW),
                    SIGNAL(clicked(bool)),
                    nullptr,
                    CBubbleTip::Close | CBubbleTip::Check | CBubbleTip::Next);
        }
};

class CTutoStartingHelperFactory : public CWizardScenarioFactory
{
    public:

        CTutoStartingHelperFactory()
        {
            m_info.m_name = "IkomiaStart";
            m_info.m_description = "First steps with Ikomia.";
            m_info.m_authors = "Guillaume Demarcq";
            m_info.m_keywords = "First, startup, Ikomia, helper";
            m_info.m_iconPath = "";
        }

        virtual CWizardScenario* create(CMainModel* pModel, CMainView* pView) override
        {
            return new CTutoStartingHelper(pModel, pView);
        }
};

#endif // CTUTOSTARTINHELPER_HPP
