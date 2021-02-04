#ifndef CPLUGINQUERYMODEL_H
#define CPLUGINQUERYMODEL_H

#include <QStandardItemModel>

class CPluginPythonModel : public QStandardItemModel
{
    public:

        enum Role
        {
            StatusRole = Qt::UserRole + 1
        };

        CPluginPythonModel(QObject *parent = Q_NULLPTR);

        void                update();

        virtual QVariant    data(const QModelIndex &index, int role) const override;

    private:

        void                initHeaderLabels();

        void                fill();
        void                fillLoadedPlugins();
        void                fillUnloadedPlugins();
        QStandardItem*      addItem(int row, int col, const QString& data);

    private:

        QSet<QString>   m_loadedPluginsFolder;
};

#endif // CPLUGINQUERYMODEL_H
