#ifndef CGRAPHICSLAYERCHOICEDLG_H
#define CGRAPHICSLAYERCHOICEDLG_H

#include "View/Common/CDialog.h"

class QTreeView;
class CProjectGraphicsProxyModel;

class CGraphicsLayerChoiceDlg : public CDialog
{
    public:

        CGraphicsLayerChoiceDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void        setModel(CProjectGraphicsProxyModel* pModel);

        QModelIndex getSelectedLayer() const;

    public slots:

        int         exec();
        void        onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
        void        accept();

    private:

        void        initLayout();
        void        initConnections();

    private:

        QTreeView*                  m_pTreeView = nullptr;
        QPushButton*                m_pBtnAccept = nullptr;
        CProjectGraphicsProxyModel* m_pModel = nullptr;
        QModelIndex                 m_selectedLayerIndex;
};

#endif // CGRAPHICSLAYERCHOICEDLG_H
