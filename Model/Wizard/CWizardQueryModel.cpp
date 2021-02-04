#include "CWizardQueryModel.h"

CWizardQueryModel::CWizardQueryModel(QObject* parent) : QSqlQueryModel(parent)
{

}

QVariant CWizardQueryModel::data(const QModelIndex& index, int role) const
{
    if(role == Qt::DecorationRole)
        return QIcon(":/Images/logo.png");

    return QSqlQueryModel::data(index, role);
}
