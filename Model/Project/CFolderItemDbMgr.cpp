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

#include "CFolderItemDbMgr.h"
#include "Model/Project/CFolderItem.hpp"

CFolderItemDbMgr::CFolderItemDbMgr() : CProjectItemBaseDbMgr()
{
}

CFolderItemDbMgr::CFolderItemDbMgr(const QString &path, const QString &connection) : CProjectItemBaseDbMgr(path, connection)
{
}

std::shared_ptr<CItem> CFolderItemDbMgr::load(const QSqlQuery &q, QModelIndex &previousIndex)
{
    Q_UNUSED(previousIndex);
    auto name = q.record().value("name").toString().toStdString();
    return std::make_shared<CFolderItem>(name);
}

void CFolderItemDbMgr::save(std::shared_ptr<ProjectTreeItem> itemPtr, int dbId)
{
    Q_UNUSED(itemPtr);
    Q_UNUSED(dbId);
}

void CFolderItemDbMgr::batchSave()
{
}
