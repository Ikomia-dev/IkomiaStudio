#include "CWizardStepModel.h"
#include "CWizardScenario.h"

CWizardStepModel::CWizardStepModel(const QList<WizardStep> stepList, QObject* parent) : QStandardItemModel(parent)
{
    int row = 0;
    for(auto&& it : stepList)
    {
        CWizardItem* pItem = new CWizardItem(it.text);
        setItem(row++, pItem);
        pItem->setCheckState(Qt::Unchecked);
        if(row == stepList.size())
            pItem->setType(QStandardItem::UserType);
    }
}
