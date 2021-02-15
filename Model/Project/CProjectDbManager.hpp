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

#ifndef CPROJECTDATABASE_H
#define CPROJECTDATABASE_H

#include "CTreeDbManager.hpp"
#include "CProjectModel.h"
#include <boost/filesystem.hpp>
#include "CProjectDbMgrRegistration.h"

class CProjectDbManager : public CTreeDbManager<CProjectModel, std::shared_ptr<CProjectModel::TreeItem>>
{
    public:

        // Ctor
        CProjectDbManager(CProjectModel* pModel)
        {
            m_pModel = pModel;
            if(m_pModel)
                m_projectFileName = m_pModel->getPath();
        }
        CProjectDbManager(QString projectName, CProjectModel* pModel)
        {
            m_projectName = projectName;
            m_pModel = pModel;
        }

        QString getPath() const
        {
            return m_projectFileName;
        }

        // Main functions
        void    newProject()
        {
            createDb("QSQLITE", m_projectFileName);
            createTree("project");
            createPropertiesTable();
            setTreeRoot(m_projectName);
        }

        void    loadProject(const QString& fileName)
        {
            m_projectFileName = fileName;
            createDb("QSQLITE", m_projectFileName);
            loadProperties();

            if(m_bHasTransaction)
                m_db.transaction();

            //m_error = activatePragma();
            fillModelFromDatabase();

            if(m_bHasTransaction)
                m_db.commit();

            if(m_pModel->isExported() == true && m_pModel->getPath() != m_pModel->getOriginalPath())
                updateOriginalPath();
        }

        void    saveProject(const QString& fileName)
        {
            m_projectFileName = fileName;
            m_pModel->setOriginalPath(fileName);

            createDb("QSQLITE", m_projectFileName);
            createPropertiesTable();
            saveProperties();

            if(m_bHasTransaction)
                m_db.transaction();

            //activatePragma();
            fillDatabaseFromModel("project");

            if(m_bHasTransaction)
                m_db.commit();
        }

        void    setProjectFileName(const QString& fileName)
        {
            m_projectFileName = fileName;
        }

    private:

        template<typename T>
        void    addProjectItem(QSqlQuery q, ProjectItemDbMgrPtr itemPtr)
        {
            assert(q.isValid());
            int nodeLevel = q.record().value("level").toInt();
            int nodeId = q.record().value("id").toInt();
            int parentId = q.record().value("parentid").toInt();
            auto nodeItem = std::static_pointer_cast<T>(itemPtr->load(q, m_prevIndex));
            assert(nodeItem != nullptr);

            // Liste des éléments de niveau immédiatement inférieur
            itemMap listPreviousLevel = m_map.value(nodeLevel - 1);

            if (listPreviousLevel.isEmpty() == false)
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
        }

        void    saveItem(std::shared_ptr<ProjectTreeItem> itemPtr)
        {
            assert(itemPtr);

            ProjectItemDbMgrPtr itemDbMgrPtr;
            int itemDbId = addItem(QString::fromStdString(itemPtr->getName()), itemPtr->getTypeId(), itemPtr->getId(), itemPtr->getParent()->getId());
            auto typeId = static_cast<TreeItemType>(itemPtr->getTypeId());
            auto it = m_itemDbManagers.find(typeId);

            if(it == m_itemDbManagers.end())
            {
                auto dbMgrFactory = m_dbMgrRegistration.getFactory();
                itemDbMgrPtr = dbMgrFactory.createObject(typeId, getPath(), getConnectionName());
                m_itemDbManagers.insert(typeId, itemDbMgrPtr);
            }
            else
                itemDbMgrPtr = it.value();

            itemDbMgrPtr->save(itemPtr, itemDbId);

            for(int i = 0; i < itemPtr->getChildCount(); ++i)
            {
                auto childItemPtr = itemPtr->getChild(i);
                saveItem(childItemPtr);
            }
        }
        void    saveProperties()
        {
            QString path = QString::fromStdString(Utils::String::dbFormat(m_pModel->getOriginalPath().toStdString()));

            QSqlQuery q(m_db);
            if(!q.exec(QString("INSERT INTO projectProperties(name, value) VALUES ('%1', '%2');").arg("originalPath").arg(path)))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

            if(!q.exec(QString("INSERT INTO projectProperties(name, value) VALUES('%1', %2);").arg("isExported").arg(m_pModel->isExported())))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
        }

        void    batchSave()
        {
            for(auto it=m_itemDbManagers.begin(); it!=m_itemDbManagers.end(); ++it)
                it.value()->batchSave();
        }

        // Reimplemented methods
        void    fillModel(QSqlQuery query) override
        {
            while (query.next())
            {
                ProjectItemDbMgrPtr itemDbMgrPtr;
                TreeItemType typeId = static_cast<TreeItemType>(query.record().value("typeid").toInt());

                auto it = m_itemDbManagers.find(typeId);
                if(it == m_itemDbManagers.end())
                {
                    auto dbMgrFactory = m_dbMgrRegistration.getFactory();
                    itemDbMgrPtr = dbMgrFactory.createObject(typeId, getPath(), getConnectionName());
                    itemDbMgrPtr->setProjectProperties(m_pModel->getPath(), m_pModel->getOriginalPath(), m_pModel->isExported());
                    m_itemDbManagers.insert(typeId, itemDbMgrPtr);
                }
                else
                    itemDbMgrPtr = it.value();

                if(itemDbMgrPtr == nullptr)
                    throw CException(CoreExCode::NULL_POINTER, "Fail to create project item database manager", __func__, __FILE__, __LINE__);

                switch(typeId)
                {
                    case TreeItemType::PROJECT : addProjectItem<CProjectItem>(query, itemDbMgrPtr); break;
                    case TreeItemType::FOLDER : addProjectItem<CFolderItem>(query, itemDbMgrPtr); break;
                    case TreeItemType::DATASET: addProjectItem<CDatasetItem<CMat>>(query, itemDbMgrPtr); break;
                    case TreeItemType::DIMENSION: addProjectItem<CDimensionItem>(query, itemDbMgrPtr); break;
                    case TreeItemType::IMAGE: addProjectItem<CImageItem>(query, itemDbMgrPtr); break;
                    case TreeItemType::GRAPHICS_LAYER: addProjectItem<CGraphicsLayerItem>(query, itemDbMgrPtr); break;
                    case TreeItemType::RESULT: addProjectItem<CResultItem>(query, itemDbMgrPtr); break;
                    case TreeItemType::VIDEO: addProjectItem<CVideoItem>(query, itemDbMgrPtr); break;
                    case TreeItemType::LIVE_STREAM: addProjectItem<CLiveStreamItem>(query, itemDbMgrPtr); break;
                    case TreeItemType::PROCESS:
                    case TreeItemType::NONE : break;
                }
            }
        }
        void    fillDatabase(std::shared_ptr<CProjectModel::TreeItem> root) override
        {
            assert(root != nullptr);

            for(int i = 0; i<root->getChildCount(); ++i)
            {
                auto childItemPtr = root->getChild(i);
                saveItem(childItemPtr);
            }
            batchSave();
        }

        void    createPropertiesTable()
        {
            QSqlQuery q(m_db);

            // Test if the table exists or not
            auto tables = m_db.tables();
            if(tables.contains("projectProperties"))
            {
                if(!q.exec(QString("DELETE FROM projectProperties")))
                    throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
            }
            else
            {
                // Create table
                if (!q.exec(QString("CREATE TABLE projectProperties(id INTEGER PRIMARY KEY, name TEXT NOT NULL, value TEXT NOT NULL);")))
                    throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);
            }
        }

        void    loadProperties()
        {
            // Test if the table exists or not
            auto tables = m_db.tables();
            if(tables.contains("projectProperties"))
            {
                QSqlQuery q(m_db);
                if(!q.exec("SELECT name, value FROM projectProperties;"))
                    throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

                QMap<QString, QString> properties;
                while(q.next())
                    properties.insert(q.value(0).toString(), q.value(1).toString());

                m_pModel->setOriginalPath(properties["originalPath"]);
                m_pModel->setExported(properties["isExported"].toInt());
            }
        }

        void    updateOriginalPath()
        {
            QSqlQuery q(m_db);
            if(!q.exec(QString("UPDATE projectProperties SET value = '%1' WHERE name = '%2';").arg(m_projectFileName).arg("originalPath")))
                throw CException(DatabaseExCode::INVALID_QUERY, q.lastError().text().toStdString(), __func__, __FILE__, __LINE__);

            m_pModel->setOriginalPath(m_projectFileName);
        }

    private:

        QString                                 m_projectName = "Project";
        QString                                 m_projectFileName = ":memory:";
        QMap<TreeItemType, ProjectItemDbMgrPtr> m_itemDbManagers;
        CProjectDbMgrRegistration               m_dbMgrRegistration;
};

#endif // CPROJECTDATABASE_H
