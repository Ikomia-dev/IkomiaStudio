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

#ifndef CMAINMODEL_H
#define CMAINMODEL_H

#include <QObject>
#include "Main/AppDefine.hpp"
#include "Project/CProjectManager.h"
#include "Plugin/CPluginManager.h"
#include "Process/CProcessManager.h"
#include "Workflow/CWorkflowManager.h"
#include "Render/CRenderManager.h"
#include "ProgressBar/CProgressBarManager.h"
#include "Graphics/CGraphicsManager.h"
#include "Results/CResultManager.h"
#include "Model/Data/Video/CVideoManager.h"
#include "User/CUserManager.h"
#include "Model/Data/CMainDataManager.h"
#include "Model/Hub/CHubManager.h"
#include "Settings/CSettingsManager.h"
#include "Model/CDbManager.h"

/**
 * @brief
 *
 */
class CMainModel : public QObject
{
    Q_OBJECT

    public:

        /**
         * @brief
         *
         */
        CMainModel();
        ~CMainModel();

        //Getters
        CProjectManager*        getProjectManager();
        CProcessManager*        getProcessManager();
        CWorkflowManager*       getWorkflowManager();
        CRenderManager*         getRenderManager();
        CProgressBarManager*    getProgressManager();
        CGraphicsManager*       getGraphicsManager();
        CResultManager*         getResultManager();
        CUserManager*           getUserManager();
        CMainDataManager*       getDataManager();
        CHubManager*            getHubManager();
        CSettingsManager*       getSettingsManager();
        CPluginManager*         getPluginManager();

        void                    init();

        void                    notifyViewShow();

    signals:

        void                    doSetSplashMessage(const QString &message, int alignment, const QColor &color);

    public slots:

        void                    onOpenImage(const QModelIndex& index);
        void                    onSetCurrentUser(const CUser& user);
        void                    onStartJupyterLab();

    private:

        void                    initConnections();
        void                    initLogFile();
        void                    initDb();
        void                    initNetworkManager();
        void                    initProjectManager();
        void                    initProcessManager();
        void                    initWorkflowManager();
        void                    initGraphicsManager();
        void                    initResultsManager();
        void                    initRenderManager();
        void                    initDataManager();
        void                    initUserManager();
        void                    initHubManager();
        void                    initSettingsManager();
        void                    initPluginManager();
        void                    initPython();
        void                    initMatomo();

        void                    writeLogMsg(int type, const QString &msg, const QString &categoryName);

        void                    checkUserInstall();

        void                    installPythonRequirements();

    private:

        CDbManager              m_dbMgr;
        CProjectManager         m_projectMgr;
        CProcessManager         m_processMgr;
        CWorkflowManager        m_workflowMgr;
        CRenderManager          m_renderMgr;
        CProgressBarManager     m_progressMgr;
        CGraphicsManager        m_graphicsMgr;
        CResultManager          m_resultsMgr;
        CUserManager            m_userMgr;
        CMainDataManager        m_dataMgr;
        CHubManager             m_hubMgr;
        CSettingsManager        m_settingsMgr;
        CPluginManager          m_pluginMgr;
        QNetworkAccessManager   m_networkMgr;
        std::string             m_logFilePath;
};

#endif // CMAINMODEL_H
