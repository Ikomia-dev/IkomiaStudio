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

#ifndef CPROCESSMANAGER_H
#define CPROCESSMANAGER_H

#include <QObject>
#include "CProcessModel.hpp"
#include "CTreeDbManager.hpp"
#include "Core/CIkomiaRegistry.h"
#include "Model/User/CUser.h"


class CPluginManager;
class CWorkflowManager;


class CProcessTableModel : public QSqlTableModel
{
    public:

        CProcessTableModel(QObject *parent = Q_NULLPTR, QSqlDatabase db = QSqlDatabase()) : QSqlTableModel(parent, db)
        {
        }

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
        {
            if (role == Qt::DecorationRole)
            {
                auto iconPath = record(index.row()).value("iconPath").toString();
                QIcon icon(iconPath);
                if(!icon.isNull())
                    return icon;
            }
            return QSqlTableModel::data(index, role);
        }
};

class CProcessProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

    public:

        CProcessProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent){}

    protected:

        virtual bool filterAcceptsRow(int source_row, const QModelIndex & source_parent) const override
        {
            // check the current item
            bool result = false;
            QModelIndex currIndex = sourceModel()->index(source_row, 0, source_parent);
            auto pItem = static_cast<CProcessModel::TreeItem*>(currIndex.internalPointer());
            size_t typeId = pItem->getTypeId();
            if(typeId == TreeItemType::PROCESS)
                result = true;

            if (sourceModel()->hasChildren(currIndex))
            {
                // if it has sub items
                for (int i = 0; i < sourceModel()->rowCount(currIndex) && !result; ++i)
                {
                    // keep the parent if a children is shown
                    result = result || filterAcceptsRow(i, currIndex);
                }
            }
            return result;
        }
};

class CProcessTableProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

    public:

        CProcessTableProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent){}

        void setProcessModel(CProcessModel* pProcessModel)
        {
            m_pProcessModel = pProcessModel;
        }

    protected:

        virtual QVariant data(const QModelIndex &index, int role) const
        {
            // For displaying correct process icon
            if (role == Qt::DecorationRole)
            {
                // Map index to source model
                auto srcIndex = mapToSource(index);
                // Get QSqlTableModel
                auto pTable = static_cast<const QSqlTableModel*>(srcIndex.model());
                // Get name from table process (id, name, description, folder_id)
                QString name = pTable->record(index.row()).value("name").toString();
                // Get index in CProcessModel from name (unicity?)
                auto ind = m_pProcessModel->getIndexFrom(m_pProcessModel->getRoot(), name.toStdString());
                return m_pProcessModel->data(ind, role);
            }
            return QSortFilterProxyModel::data(index, role);
        }

    private:

        CProcessModel* m_pProcessModel = nullptr;
};

class CProcessManager: public QObject, public CTreeDbManager<CProcessModel, std::shared_ptr<ProcessTreeItem>>
{
    Q_OBJECT

    public:

        enum class ID : size_t
        {
            PROCESS_TREE,
            PROCESS_POPUP
        };
        using ProcessModelList = std::map<ID, std::shared_ptr<CProcessModel>>;
        using ProcessProxyModelList = std::map<ID, QSortFilterProxyModel*>;
        using ProcessTableModelList = std::map<ID, std::shared_ptr<CProcessTableModel>>;
        using ProcessProxyTableModelList = std::map<ID, QSortFilterProxyModel*>;

        CProcessManager();
        ~CProcessManager();

        void                    init();

        void                    setManagers(CPluginManager* pPluginMgr, CWorkflowManager* pWorkflowMgr);
        void                    setCurrentUser(const CUser& user);

        CProcessModel*          getProcessModel(CProcessManager::ID id);
        CTaskInfo               getProcessInfo(const std::string& processName) const;

        void                    notifyViewShow();

        WorkflowTaskPtr         createObject(const std::string& processName, const WorkflowTaskParamPtr &paramPtr);
        WorkflowTaskWidgetPtr   createWidget(const std::string& processName, const WorkflowTaskParamPtr &paramPtr);

        bool                    reloadAll();
        bool                    reloadPlugin(const QString& pluginName, int language);

    signals:

        void                    doSetProcessModel(QSortFilterProxyModel* pModel);
        void                    doSetTableModel(QSortFilterProxyModel* pModel);
        void                    doSetWidgetInstance(const std::string& processName, WorkflowTaskWidgetPtr& widgetPtr);
        void                    doSetProcessInfo(const CTaskInfo& info);
        void                    doOnAllProcessReloaded();
        void                    doOnProcessReloaded(const QString& name);

    public slots:

        void                    onReloadAllPlugins();
        void                    onReloadPlugin(const QString &pluginName, int language);
        void                    onSearchProcess(const QString& text);
        void                    onSetCurrentProcessModel(ID id);
        void                    onUpdateTableModel(const QModelIndex& index);
        void                    onSearchTableProcess(const QString& text);
        void                    onQueryWidgetInstance(const std::string& processName);
        void                    onQueryProcessInfo(const std::string& processName);
        void                    onUpdateProcessInfo(bool bFullEdit, const CTaskInfo& info);
        void                    onInstallPluginRequirements(const QString& name);

    protected:

        void                    fillModel(QSqlQuery query) override;

        void                    updateModelFromSearch(const QString& text);

        void                    createCustomTable();

        void                    fillDatabase(std::shared_ptr<ProcessTreeItem> root) override;

        void                    setCurrentProcessModel(ID id);

    private:

        class CItemImpl
        {
            public:

                void                            setModel(CProcessModel* pModel){ m_pModel = pModel; }
                void                            setIndex(QModelIndex index) { m_index = index; }
                void                            setDbManager(CTreeDbManager* pParent) { m_pMgr = static_cast<CProcessManager*>(pParent); }

            protected:

                CProcessManager*    m_pMgr = nullptr;
                CProcessModel*      m_pModel = nullptr;
                QModelIndex         m_index;
        };

        class CFolderItemImpl : public CItemImpl
        {
            public:

                std::shared_ptr<CFolderItem>    load(QSqlQuery q)
                {
                    auto id = q.record().value("id").toInt();
                    auto name = q.record().value("name").toString().toStdString();

                    auto db = QSqlDatabase::database("ProcessConnection");
                    QSqlQuery query(db);
                    if(!query.exec(QString("SELECT processCount, iconPath FROM processFolder WHERE id=%1").arg(id)))
                        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

                    auto pItem = std::make_shared<CFolderItem>(name);
                    if(query.next())
                    {
                        auto strProcessCount = query.value(0).toString().toStdString();
                        name += "(" + strProcessCount + ")";
                        pItem->setName(name);

                        auto strIconPath = query.value(1).toString();
                        if(!strIconPath.isEmpty())
                        {
                            QPixmap icon(strIconPath);
                            pItem->setIconPixmap(icon);
                        }
                    }
                    return pItem;
                }
                void                            save(std::shared_ptr<ProcessTreeItem> /*item*/)
                {
                }
        };

        class CProcessItemImpl : public CItemImpl
        {
            public:

                std::shared_ptr<CProcessItem>   load(QSqlQuery q)
                {
                    auto id = q.record().value("id").toInt();
                    auto name = q.record().value("name").toString().toStdString();

                    auto db = QSqlDatabase::database("ProcessConnection");
                    QSqlQuery query(db);
                    if(!query.exec(QString("select iconPath from process where id=%1").arg(id)))
                        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

                    if(!query.next())
                        throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

                    auto iconPath = query.value(0).toString();
                    auto pItem = std::make_shared<CProcessItem>(name);

                    if(!iconPath.isEmpty())
                    {
                        QPixmap icon(iconPath);
                        pItem->setIconPixmap(icon);
                    }
                    return pItem;
                }
                void                            save(std::shared_ptr<ProcessTreeItem> /*item*/)
                {
                }
        };

        // Template functions
        template<typename T, typename Loader>
        auto                    addCustomTreeItem(QSqlQuery q)
        {
            assert(q.isValid());

            int nodeLevel = q.record().value("level").toInt();
            int nodeId = q.record().value("id").toInt();
            int parentId = q.record().value("parentid").toInt();

            auto nodeItem = loadCustomTreeItem<T, Loader>(q);
            assert(nodeItem != nullptr);
            nodeItem->setDbId(nodeId);

            // Liste des éléments de niveau immédiatement inférieur
            itemMap listPreviousLevel = m_map.value(nodeLevel - 1);

            if (!listPreviousLevel.isEmpty())
            {
                auto parent = listPreviousLevel.value(parentId);
                assert(parent != nullptr);
                parent->emplace_back(nodeItem);
                m_prevIndex = m_pModel->getIndexFrom(parent, nodeItem);
                // Ajout de l'élément à la liste des éléments de même niveau
                itemMap listCurrentLevel = m_map.value(nodeLevel);
                listCurrentLevel.insert(nodeId, parent->getLastChild());
                m_map.insert(nodeLevel, listCurrentLevel);
            }
            else
            {
                // Cet élément est le premier dans son niveau : on l'ajoute directement à la racine
                auto root = m_pModel->getRoot();
                root->emplace_back(nodeItem);
                m_prevIndex = m_pModel->getIndexFrom(root, nodeItem);
                listPreviousLevel.insert(nodeId, root->getLastChild());
                m_map.insert(nodeLevel, listPreviousLevel);
            }
            return nodeItem;
        }

        void                    initModel();

        void                    resetModel();

        void                    createModel();
        void                    createProcessTable(QSqlDatabase& db);
        void                    createFolderTable(QSqlDatabase& db);
        void                    createCustomTreeFolders();

        size_t                  buildPath(const std::string& path);

        size_t                  addFolder(const std::string& folder, const std::string & path, size_t parentId);
        size_t                  addProcess(const TaskFactoryPtr& process, size_t folderId);
        void                    addProcessInfo(const TaskFactoryPtr &process, size_t id, size_t folderId);
        void                    addFoldersInfo();
        void                    addCustomPathIcon(const std::string& path, const std::string& iconPath);

        void                    notifyModelUpdate();
        void                    notifyTableModelUpdate();

        void                    updateTableModelQuery();
        void                    updateProcessInfo(const CTaskInfo& info, bool bFull=true);

        void                    syncTaskInfo();

    public:

        CIkomiaRegistry         m_registry;

    private:

        using ProcessPathInfo = std::map<std::string, std::pair<size_t,size_t>>;
        using MapString = std::map<std::string, std::string>;

        QString                     m_dbType = "QSQLITE";
        QString                     m_dbName = ":memory:";
        ProcessModelList            m_processList;
        ProcessProxyModelList       m_proxyList;
        ProcessTableModelList       m_processTableList;
        ProcessProxyTableModelList  m_proxyTableList;
        CProcessTableProxyModel*    m_pTableProxyModel = nullptr;
        ProcessPathInfo             m_pathMap;
        CPluginManager*             m_pPluginMgr = nullptr;
        CWorkflowManager*           m_pWorkflowMgr = nullptr;
        unsigned int                m_id = 0;
        ID                          m_processId = ID::PROCESS_TREE;
        const std::vector<ID>       m_viewIds = {ID::PROCESS_TREE, ID::PROCESS_POPUP};
        bool                        m_bIsSearching = false;
        QString                     m_searchReq;
        int                         m_processCount = 0;
        MapString                   m_iconMap;
};

#endif // CPROCESSMANAGER_H
