#ifndef CUSERMANAGEMENTWIDGET_H
#define CUSERMANAGEMENTWIDGET_H

#include <QObject>
#include <QWidget>

class QSqlQueryModel;

class CUserManagementWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CUserManagementWidget(QWidget *parent = nullptr);

        void            setModel(QSqlQueryModel* pModel);
        void            setCurrentUserInfo(int id, int role);

        virtual QSize   sizeHint() const override;

    signals:

        void            doAddUser(const QString& fullname, const QString& login, const QString& pwd, int role);
        void            doDeleteUser(int id);
        void            doModifyUser(int id, const QString& fullname, const QString& login, const QString& pwd, int role);

    private slots:

        void            onNewUser();
        void            onEditUser();
        void            onDeleteUser();
        void            onTableViewClicked(const QModelIndex& index);

    private:

        void            initLayout();
        void            initConnections();

        QPushButton*    createButton(const QString& text, const QIcon& icon);

        void            enableAdministration(bool bEnable);

    private:

        int             m_currentUserId = -1;
        int             m_currentUserRole = -1;
        QTableView*     m_pUserTable = nullptr;
        QPushButton*    m_pBtnNew = nullptr;
        QPushButton*    m_pBtnEdit = nullptr;
        QPushButton*    m_pBtnDelete = nullptr;
};

#endif // CUSERMANAGEMENTWIDGET_H
