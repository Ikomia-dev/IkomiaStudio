#ifndef CMEASURESTABLEMODEL_H
#define CMEASURESTABLEMODEL_H

#include <QSqlQueryModel>

class CMeasuresTableModel : public QSqlQueryModel
{
    public:

        CMeasuresTableModel(QObject* parent=Q_NULLPTR);
};

#endif // CMEASURESTABLEMODEL_H
