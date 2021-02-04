#ifndef CPROTOCOLDBMANAGER_H
#define CPROTOCOLDBMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include "Core/CProtocol.h"

class CProcessManager;
class CSettingsManager;
class CGraphicsContext;
using GraphicsContextPtr = std::shared_ptr<CGraphicsContext>;

class CProtocolDBManager
{
    public:

        CProtocolDBManager();
        ~CProtocolDBManager();

        void                        setManagers(CSettingsManager *pSettingsMgr);

        std::map<QString, int>      getProtocols();
        QString                     getProtocolDescription(const int protocolId);

        QStringList                 searchProtocols(const QString& text);

        int                         save(const ProtocolPtr &pProtocol);
        int                         save(const ProtocolPtr& pProtocol, QString path);

        std::unique_ptr<CProtocol>  load(int protocolId, CProcessManager* pProcessMgr, const GraphicsContextPtr& graphicsContextPtr);
        std::unique_ptr<CProtocol>  load(QString path, CProcessManager* pProcessMgr, const GraphicsContextPtr& graphicsContextPtr);

        void                        remove(int protocolId);

    private:

        QSqlDatabase                initDB(const QString &path, const QString &connectionName);

        void                        createTables(QSqlDatabase &db);

        UMapString                  getTaskParameters(QSqlDatabase &db, int taskId);

        int                         save(QSqlDatabase &db, const ProtocolPtr &pProtocol);

        std::unique_ptr<CProtocol>  load(QSqlDatabase &db, int protocolId, CProcessManager* pProcessMgr, const GraphicsContextPtr& graphicsContextPtr);

    private:

        QString             m_path;
        CSettingsManager*   m_pSettingsMgr = nullptr;
};

#endif // CPROTOCOLDBMANAGER_H
