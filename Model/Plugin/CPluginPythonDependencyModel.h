#ifndef CPLUGINPYTHONDEPENDENCYMODEL_H
#define CPLUGINPYTHONDEPENDENCYMODEL_H

#include <QStandardItemModel>

class CPluginPythonDependencyModel: public QStandardItemModel
{
    public:

        enum Role
        {
            StatusRole = Qt::UserRole + 1
        };

        CPluginPythonDependencyModel(QObject *parent = nullptr);

        void                setPluginName(const QString& name,
                                          const QMap<QString,QPair<QString,QString>>& allPackages,
                                          const QMap<QString,QString>& aliases);

        virtual QVariant    data(const QModelIndex &index, int role) const override;

    private:

        void                addModule(const QString& name, const QString version, const QString lastVersion, bool bMissing);
};

#endif // CPLUGINPYTHONDEPENDENCYMODEL_H
