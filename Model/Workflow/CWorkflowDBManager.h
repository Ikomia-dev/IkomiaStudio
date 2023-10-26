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

#ifndef CWORKFLOWDBMANAGER_H
#define CWORKFLOWDBMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include "Core/CWorkflow.h"

class CProcessManager;
class CSettingsManager;
class CGraphicsContext;
using GraphicsContextPtr = std::shared_ptr<CGraphicsContext>;

class CWorkflowDBManager
{
    public:

        CWorkflowDBManager();
        ~CWorkflowDBManager();

        void                        setManagers(CSettingsManager *pSettingsMgr);

        std::map<QString, int>      getWorkflows();
        QString                     getWorkflowDescription(const int protocolId);
        void                        getWorkflowInfo(const int protocolId, QString& description, QString& keywords);

        QStringList                 searchWorkflows(const QString& text);

        int                         save(const WorkflowPtr &pWorkflow);
        int                         save(const WorkflowPtr& pWorkflow, QString path);

        std::unique_ptr<CWorkflow>  load(int protocolId, CProcessManager* pProcessMgr, const GraphicsContextPtr& graphicsContextPtr);
        std::unique_ptr<CWorkflow>  load(QString path, CProcessManager* pProcessMgr, const GraphicsContextPtr& graphicsContextPtr);

        void                        remove(int protocolId);

    private:

        QSqlDatabase                initDB(const QString &path, const QString &connectionName);

        void                        createTables(QSqlDatabase &db);

        UMapString                  getTaskParameters(QSqlDatabase &db, int taskId);

        int                         save(QSqlDatabase &db, const WorkflowPtr &pWorkflow);

        std::unique_ptr<CWorkflow>  load(QSqlDatabase &db, int protocolId, CProcessManager* pProcessMgr, const GraphicsContextPtr& graphicsContextPtr);

    private:

        QString             m_path;
        CSettingsManager*   m_pSettingsMgr = nullptr;
};

#endif // CWORKFLOWDBMANAGER_H
