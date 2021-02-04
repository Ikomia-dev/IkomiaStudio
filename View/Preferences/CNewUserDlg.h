#ifndef CNEWUSERWIDGET_H
#define CNEWUSERWIDGET_H

class QSqlQueryModel;

class CNewUserDlg : public QDialog
{
    Q_OBJECT

    public:

        explicit CNewUserDlg(QSqlQueryModel* pModel, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        QString getFullName() const;
        QString getLogin() const;
        QString getPassword() const;
        int     getRole() const;

        bool    isPasswordModified() const;

        void    setUserInfo(int id, const QString& fullName, const QString& login, const QString& pwd, int role);

    private:

        void    initLayout();
        void    initConnections();

        QFrame* createLine();

        void    messageBox(const QString& text);

        bool    checkUnicity() const;

    private slots:

        void    onOK();

    private:

        QSqlQueryModel* m_pModel = nullptr;
        QLineEdit*      m_pEditFullName = nullptr;
        QLineEdit*      m_pEditLogin = nullptr;
        QLineEdit*      m_pEditPwd = nullptr;
        QLineEdit*      m_pEditConfirmPwd = nullptr;
        QComboBox*      m_pComboRole = nullptr;
        QPushButton*    m_pBtnOk = nullptr;
        QPushButton*    m_pBtnCancel = nullptr;
        int             m_userId = -1;
        bool            m_bPwdModified = false;
};

#endif // CNEWUSERWIDGET_H
