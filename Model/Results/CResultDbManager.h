#ifndef CRESULTDBMANAGER_H
#define CRESULTDBMANAGER_H

#include <QSqlQueryModel>
#include "Model/Project/CProjectDbMgrInterface.hpp"
#include "Model/Data/CMeasuresTableModel.h"
#include "Main/CoreDefine.hpp"
#include "IO/CMeasureProcessIO.h"

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
