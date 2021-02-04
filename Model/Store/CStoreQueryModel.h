#ifndef CSTORETABLEMODEL_H
#define CSTORETABLEMODEL_H

#include <QSqlQueryModel>
#include "Model/User/CUser.h"

class CStoreQueryModel : public QSqlQueryModel
{
    public:

        CStoreQueryModel(QObject *parent = Q_NULLPTR);

        QVariant    data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        void        setCurrentUser(const CUser& user);

        CUser       getCurrentUser() const;

    private:

        CUser       m_currentUser;
};

#endif // CSTORETABLEMODEL_H
