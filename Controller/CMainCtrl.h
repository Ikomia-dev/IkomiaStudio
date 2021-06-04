/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CMAINCTRL_H
#define CMAINCTRL_H

#include <QObject>
#include "View/Main/CMainView.h"
#include "Model/CMainModel.h"
#include "Model/Wizard/CWizardManager.h"

/**
 * @brief
 *
 */
class CMainCtrl : public QObject
{
    public:

        /**
         * @brief
         *
         * @param pModel
         * @param pView
         * @param pSplash
         */
        CMainCtrl(CMainModel* pModel, CMainView* pView, QSplashScreen* pSplash);

        void    show();
        void    launchWizard();

    private:

        void    initSplash();
        void    initConnections();
        void    initProjectConnections();
        void    initProcessConnections();
        void    initWorkflowConnections();
        void    initInfoConnections();
        void    initRenderConnections();
        void    initProgressConnections();
        void    initGraphicsConnections();
        void    initResultsConnections();
        void    initVideoConnections();
        void    initUserConnections();
        void    initDataConnections();
        void    initStoreConnections();
        void    initWizardConnections();
        void    initSettingsConnections();
        void    initPluginConnections();

    private:

        CMainModel*         m_pModel = nullptr;
        CMainView*          m_pView = nullptr;
        QSplashScreen*      m_pSplash = nullptr;

        // Tutorial manager
        CWizardManager      m_tutorialMgr;
};

#endif // CMAINCTRL_H
