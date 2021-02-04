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
