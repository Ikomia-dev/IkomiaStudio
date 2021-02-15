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

#ifndef CPROJECTEXPORT_H
#define CPROJECTEXPORT_H

class CProjectModel;

class CProjectExportMgr
{
    public:

        CProjectExportMgr(CProjectModel* pModel, const QString& folder);

        void    run();

    private:

        void        copy(const QModelIndex &srcIndex, const QModelIndex &dstIndex);
        QModelIndex addItem(const QModelIndex& srcIndex, const QModelIndex& dstParent);

    private:

        CProjectModel*  m_pSrcModel = nullptr;
        CProjectModel*  m_pNewModel = nullptr;
        std::string     m_parentFolder;
        std::string     m_projectFolder;
        std::string     m_dataFolder;
};

#endif // CPROJECTEXPORT_H
