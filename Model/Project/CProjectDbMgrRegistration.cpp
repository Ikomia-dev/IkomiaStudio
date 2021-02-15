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

#include <QString>
#include "CProjectDbMgrRegistration.h"
#include "CProjectItemDbMgr.h"
#include "CFolderItemDbMgr.h"
#include "CDatasetItemDbMgr.h"
#include "CDimensionItemDbMgr.h"
#include "Model/Data/Image/CImageItemDbMgr.h"
#include "Model/Graphics/CGraphicsDbManager.h"
#include "Model/Results/CResultDbManager.h"
#include "Model/Data/Video/CVideoItemDbMgr.h"
#include "Model/Data/Video/CLiveStreamItemDbMgr.h"

CProjectDbMgrRegistration::CProjectDbMgrRegistration()
{
    registerManager(std::make_shared<CProjectItemDbMgrFactory>());
    registerManager(std::make_shared<CFolderItemDbMgrFactory>());
    registerManager(std::make_shared<CDatasetItemDbMgrFactory>());
    registerManager(std::make_shared<CDimensionItemDbMgrFactory>());
    registerManager(std::make_shared<CImageItemDbMgrFactory>());
    registerManager(std::make_shared<CGraphicsDbMgrFactory>());
    registerManager(std::make_shared<CResultDbMgrFactory>());
    registerManager(std::make_shared<CVideoItemDbMgrFactory>());
    registerManager(std::make_shared<CLiveStreamItemDbMgrFactory>());
}

const CProjectDbMgrAbstractFactory &CProjectDbMgrRegistration::getFactory() const
{
    return m_factory;
}

void CProjectDbMgrRegistration::registerManager(const std::shared_ptr<CProjectDbMgrFactory>& factoryPtr)
{
    assert(factoryPtr);

    m_factory.getList().push_back(factoryPtr);
    //Passage par lambda -> factoryPtr par valeur pour assurer la portée du pointeur
    auto pCreatorFunc = [factoryPtr](const QString& path, const QString& connection)
    {
        return factoryPtr->create(path, connection);
    };
    m_factory.registerCreator(factoryPtr->getType(), pCreatorFunc);
}
