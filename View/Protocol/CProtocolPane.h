#ifndef CPROTOCOLPANE_H
#define CPROTOCOLPANE_H

#include <QWidget>
#include "Main/forwards.hpp"
#include "View/Common/CContextMenu.hpp"

class CProtocolInfoDlg;

class CProtocolPane : public QWidget
{
    Q_OBJECT

    public:

        explicit CProtocolPane(QWidget *parent = nullptr);

    public slots:

        void    onSetModel(QStringListModel* pModel);
        void    onSetFromImageModel(QStringListModel* pModel);
        void    onSetDescription(const QString& text);
        void    onSearchProtocol(const QString& text);
        void    onShowContextMenu(const QPoint &pos);
        void    onShowProtocolInfo(const QModelIndex& index);
        void    onLoadSelectedProtocol();
        void    onDeleteSelectedProtocol();

    signals:

        void    doLoadProtocol(const QModelIndex& index);
        void    doDeleteProtocol(const QModelIndex& index);
        void    doSearchProtocol(const QString& text);
        void    doOpenProtocolView();
        void    doGetProtocolInfo(const QModelIndex& index);

    protected:

        bool    eventFilter(QObject *watched, QEvent *event) override;
        void    hideEvent(QHideEvent* event) override;

    private:

        void    initLayout();
        void    initContextMenu();
        void    initConnections();

        void    deleteCurrentProtocol();

        void    updateProtocolInfoPosition(QPoint itemPosition);

    private:

        QLineEdit*                  m_pSearchBox = nullptr;
        QListView*                  m_pListView = nullptr;
        QListView*                  m_pFromImageListView = nullptr;
        QTextEdit*                  m_pProtocolDesc = nullptr;
        CMultiContextMenu<menu_sig> m_contextMenu;
        CProtocolInfoDlg*           m_pInfoDlg = nullptr;
};

#endif // CPROTOCOLPANE_H
