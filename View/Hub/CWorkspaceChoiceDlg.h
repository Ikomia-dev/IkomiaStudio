#ifndef CNAMESPACECHOICEDLG_H
#define CNAMESPACECHOICEDLG_H

#include "View/Common/CDialog.h"
#include "Model/User/CUser.h"

class CWorkspaceChoiceDlg : public CDialog
{
    Q_OBJECT

    public:

        CWorkspaceChoiceDlg(const CUser& user, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        QString getWorkspacePath() const;

    private:

        void    initLayout(const CUser &user);
        void    initConnections();

    private:

        QComboBox*              m_pComboNamespaces = nullptr;
        QPushButton*            m_pOkBtn = nullptr;
        QPushButton*            m_pCancelBtn = nullptr;
};

#endif // CWORKSPACECHOICEDLG_H
