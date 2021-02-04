#ifndef CSTOREPLUGINLISTVIEW_H
#define CSTOREPLUGINLISTVIEW_H

#include <QListView>

class CStorePluginListViewDelegate;

class CStorePluginListView : public QListView
{
    Q_OBJECT

    public:

        CStorePluginListView(int pluginSource, QWidget *parent = nullptr);

    signals:

        void            doPublishPlugin(const QModelIndex& index);
        void            doInstallPlugin(const QModelIndex& index);
        void            doShowPluginInfo(const QModelIndex& index);

    protected:

        virtual void    mouseMoveEvent(QMouseEvent* event) override;

    private:

        void            initConnections();

    private:

        CStorePluginListViewDelegate*   m_pDelegate = nullptr;
};

#endif // CSTOREPLUGINLISTVIEW_H
