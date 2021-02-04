#ifndef CUSERSQLQUERYMODEL_H
#define CUSERSQLQUERYMODEL_H

#include <QSqlQueryModel>

class CUserSqlQueryModel : public QSqlQueryModel
{
    Q_OBJECT

    public:

        CUserSqlQueryModel(QObject *parent = nullptr);

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif // CUSERSQLQUERYMODEL_H
