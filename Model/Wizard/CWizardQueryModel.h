#ifndef CWIZARDQUERYMODEL_H
#define CWIZARDQUERYMODEL_H

#include <QSqlQueryModel>

class CWizardQueryModel : public QSqlQueryModel
{
    public:

        CWizardQueryModel(QObject* parent = Q_NULLPTR);

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

#endif // CWIZARDQUERYMODEL_H
