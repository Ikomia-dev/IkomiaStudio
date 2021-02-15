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

#ifndef CQTPROJECTMODEL_H
#define CQTPROJECTMODEL_H

#include "CProjectItem.hpp"
#include "Model/CTreeModel.hpp"
#include "Model/Project/CFolderItem.hpp"
#include "CDatasetItem.hpp"
#include "CDimensionItem.hpp"
#include "Model/Data/Image/CImageItem.hpp"
#include "Model/Data/Video/CVideoItem.hpp"
#include "Model/Data/Video/CLiveStreamItem.hpp"
#include "Data/CMat.hpp"
#include "Model/Graphics/CGraphicsLayerItem.hpp"
#include "Model/Results/CResultItem.hpp"

using ProjectTreeModel = CTreeModel<    std::shared_ptr<CProjectItem>,
                                        std::shared_ptr<CFolderItem>,
                                        std::shared_ptr<CDatasetItem<CMat>>,
                                        std::shared_ptr<CDimensionItem>,
                                        std::shared_ptr<CImageItem>,
                                        std::shared_ptr<CGraphicsLayerItem>,
                                        std::shared_ptr<CResultItem>,
                                        std::shared_ptr<CVideoItem>,
                                        std::shared_ptr<CLiveStreamItem>>;

using ProjectTreeItem = CTreeItem<  std::shared_ptr<CProjectItem>,
                                    std::shared_ptr<CFolderItem>,
                                    std::shared_ptr<CDatasetItem<CMat>>,
                                    std::shared_ptr<CDimensionItem>,
                                    std::shared_ptr<CImageItem>,
                                    std::shared_ptr<CGraphicsLayerItem>,
                                    std::shared_ptr<CResultItem>,
                                    std::shared_ptr<CVideoItem>,
                                    std::shared_ptr<CLiveStreamItem>>;

inline uint qHash(const ProjectTreeItem* key, uint seed = 0) Q_DECL_NOTHROW
{
    assert(key);

    QSet<uint> childrenHashes;
    childrenHashes.reserve(static_cast<int>(key->getChildCount()));
    for (int i = 0; i < key->getChildCount(); ++i)
    {
        auto node = key->getChild(i).get();
        assert(node);
        childrenHashes.insert(qHash(node, seed));
    }
    return qHash(QPair<uint, uint>(key->hash(), qHash(childrenHashes, seed)), seed);
}

class CProjectModel : public ProjectTreeModel
{
    public:

        CProjectModel(QObject *parent = 0);

        Qt::ItemFlags   flags(const QModelIndex &index) const override;

        QVariant        data(const QModelIndex &index, int role) const override;

        bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        void            setPath(const QString& path);
        void            setOriginalPath(const QString& path);
        void            setExported(bool bExported);

        QString         getPath() const;
        QString         getOriginalPath() const;

        void            updateHash();

        bool            isModified() const;
        bool            isExported() const;

    private:

        QString         m_path = "";
        QString         m_originalPath = "";
        uint            m_projectHash = 0;
        bool            m_bExported = false;
};
#endif // CQTPROJECTMODEL_H
