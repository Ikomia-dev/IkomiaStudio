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

#ifndef CRESULTDBMANAGER_H
#define CRESULTDBMANAGER_H

#include <QSqlQueryModel>
#include "Model/Project/CProjectDbMgrInterface.hpp"
#include "Model/Data/CMeasuresTableModel.h"
#include "Main/CoreDefine.hpp"
#include "IO/CMeasureIO.h"

//----------------------------------//
//----- Class CResultDBManager -----//
//----------------------------------//
class CResultDbManager : public CProjectItemBaseDbMgr
{
    public:

        CResultDbManager();
        CResultDbManager(const QString& path, const QString& connectionName);

        ~CResultDbManager();

        std::shared_ptr<CItem>  load(const QSqlQuery& q, QModelIndex& previousIndex) override;
        void                    save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId) override;
        void                    batchSave() override;

        void                    setMeasures(const ObjectsMeasures& measures, int resultDbId=-1);

        ObjectsMeasures         getMeasures(int resultId=-1);

        CMeasuresTableModel*    createMeasureModel(int resultId=-1);

        void                    remove(std::vector<int> resultIds);

    private:

        void                    initMemoryDB();

        QSqlDatabase            connectDB();

        void                    createTables();

        int                     getMeasureCount(const QSqlDatabase& db, int resultId);
        QMap<int,QString>       getMeasureNames(const QSqlDatabase& db, int resultId);

        void                    updateProjectId(int oldId, int newId);

        QString                 buildMeasureQuery(int resultId, const QMap<int,QString>& mapMeasureNames);

        QString                 encodeValues(const std::vector<double>& values);
        std::vector<double>     decodeValues(const QString &strValues);

        void                    loadTypes();

    private:

        bool            m_bTypesLoaded = false;
        QMap<int, int>  m_mapTypes;
};

class CResultDbMgrFactory: public CProjectDbMgrFactory
{
    public:

        CResultDbMgrFactory()
        {
            m_type = TreeItemType::RESULT;
        }

        ProjectItemDbMgrPtr create(const QString& path, const QString& connection)
        {
            return std::make_shared<CResultDbManager>(path, connection);
        }
};

#endif // CRESULTDBMANAGER_H
