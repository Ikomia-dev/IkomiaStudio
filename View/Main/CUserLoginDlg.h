#ifndef CUSERLOGINDLG_H
#define CUSERLOGINDLG_H

#include <QDialog>
#include "View/Common/CDialog.h"

class CUserLoginDlg : public CDialog
{
    Q_OBJECT

    public:

        CUserLoginDlg(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

        void    setCurrentUser(const QString& userName);

    signals:

        void    doConnectUser(const QString& login, const QString& pwd, bool bRememberMe);
        void    doDisconnectUser();
        void    doHide();

    private:

        void    initLayout();
        void    initConnections();
        void    hideEvent(QHideEvent* event) override;

    private:

        QLineEdit*      m_pEditLogin = nullptr;
        QLineEdit*      m_pEditPwd = nullptr;
        QCheckBox*      m_pCheckRememberMe = nullptr;
        QPushButton*    m_pBtnConnect = nullptr;
        QString         m_userName = "";
};

#endif // CUSERLOGINDLG_H
