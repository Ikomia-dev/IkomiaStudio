#ifndef CPROCESSMODEL_HPP
#define CPROCESSMODEL_HPP

#include "Model/CTreeModel.hpp"
#include "Model/Project/CFolderItem.hpp"
#include "CProcessItem.hpp"

using ProcessTreeModel = CTreeModel<    std::shared_ptr<CFolderItem>,
                                        std::shared_ptr<CProcessItem>>;

using ProcessTreeItem = CTreeItem<  std::shared_ptr<CFolderItem>,
                                    std::shared_ptr<CProcessItem>>;

class CProcessModel: public ProcessTreeModel
{
    public:

        CProcessModel(QObject *parent = Q_NULLPTR) : ProcessTreeModel(parent)
        {
        }
};

#endif // CPROCESSMODEL_HPP
