#include "CUserManagementWidget.h"
#include <QTableView>
#include "Model/User/CUserSqlQueryModel.h"
#include "Model/User/CUserManager.h"
#include "CNewUserDlg.h"

CUserManagementWidget::CUserManagementWidget(QWidget *parent) : QWidget(parent)
{
    initLayout();
    initConnections();
    enableAdministration(false);
}

void CUserManagementWidget::setModel(QSqlQueryModel *pModel)
{
    m_pUserTable->setModel(pModel);
    m_pUserTable->setColumnHidden(0, true);
    m_pUserTable->setColumnHidden(3, true);
    m_pUserTable->horizontalHeader()->setStretchLastSection(true);
}

void CUserManagementWidget::setCurrentUserInfo(int id, int role)
{
    m_currentUserId = id;
    m_currentUserRole = role;
    enableAdministration(role == CUserManager::ADMINISTRATOR);

    auto pModel = m_pUserTable->model();
    if(pModel)
    {
        int idSelect = pModel->data(pModel->index(m_pUserTable->currentIndex().row(), 0)).toInt();
        m_pBtnEdit->setEnabled(idSelect == id || role == CUserManager::ADMINISTRATOR);
    }
}

QSize CUserManagementWidget::sizeHint() const
{
    return QSize(400, 300);
}

void CUserManagementWidget::initLayout()
{
    m_pUserTable = new QTableView;
    m_pUserTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pUserTable->setSelectionMode(QAbstractItemView::SingleSelection);

    m_pBtnNew = createButton(tr("New"), QIcon(":/Images/add.png"));
    m_pBtnEdit = createButton(tr("Edit"), QIcon(":/Images/edit.png"));
    m_pBtnEdit->setEnabled(false);
    m_pBtnDelete = createButton(tr("Delete"), QIcon(":/Images/delete.png"));

    auto pVLayout = new QVBoxLayout;
    pVLayout->addWidget(m_pBtnNew);
    pVLayout->addWidget(m_pBtnEdit);
    pVLayout->addWidget(m_pBtnDelete);
    pVLayout->addStretch(1);

    auto pHLayout = new QHBoxLayout;
    pHLayout->addWidget(m_pUserTable);
    pHLayout->addLayout(pVLayout);

    setLayout(pHLayout);
}

void CUserManagementWidget::initConnections()
{
    connect(m_pBtnNew, &QPushButton::clicked, this, &CUserManagementWidget::onNewUser);
    connect(m_pBtnEdit, &QPushButton::clicked, this, &CUserManagementWidget::onEditUser);
    connect(m_pBtnDelete, &QPushButton::clicked, this, &CUserManagementWidget::onDeleteUser);
    connect(m_pUserTable, &QTableView::clicked, this, &CUserManagementWidget::onTableViewClicked);
}

QPushButton* CUserManagementWidget::createButton(const QString& text, const QIcon& icon)
{
    auto pBtn = new QPushButton;
    pBtn->setIcon(icon);
    pBtn->setToolTip(text);
    return pBtn;
}

void CUserManagementWidget::enableAdministration(bool bEnable)
{
    m_pBtnNew->setEnabled(bEnable);
    m_pBtnDelete->setEnabled(bEnable);
}

void CUserManagementWidget::onNewUser()
{
    CNewUserDlg newUserDlg(static_cast<QSqlQueryModel*>(m_pUserTable->model()), this);
    if(newUserDlg.exec() == QDialog::Accepted)
    {
        auto fullname = newUserDlg.getFullName();
        auto login = newUserDlg.getLogin();
        auto password = newUserDlg.getPassword();
        auto role = newUserDlg.getRole();
        emit doAddUser(fullname, login, password, role);
    }
}

void CUserManagementWidget::onEditUser()
{
    auto selectionModel = m_pUserTable->selectionModel();
    if(selectionModel && selectionModel->hasSelection() == true)
    {
        auto pModel = static_cast<CUserSqlQueryModel*>(m_pUserTable->model());
        auto indexes = selectionModel->selectedRows();
        assert(indexes.count() == 1);
        int row = indexes[0].row();
        int id = pModel->data(pModel->index(row, 0)).toInt();
        QString fullname = pModel->data(pModel->index(row, 1)).toString();
        QString login = pModel->data(pModel->index(row, 2)).toString();
        QString pwd = QString(pModel->data(pModel->index(row, 3)).toByteArray());
        //Call base data() method to retrieve the role as an int, not as QString (display)
        int role = pModel->QSqlQueryModel::data(pModel->index(row, 4)).toInt();

        CNewUserDlg newUserDlg(static_cast<QSqlQueryModel*>(m_pUserTable->model()), this);
        newUserDlg.setUserInfo(id, fullname, login, pwd, role);

        if(newUserDlg.exec() == QDialog::Accepted)
        {
            int id = pModel->data(pModel->index(row, 0)).toInt();
            auto fullname = newUserDlg.getFullName();
            auto login = newUserDlg.getLogin();
            auto password = newUserDlg.isPasswordModified() == false ? "" : newUserDlg.getPassword();
            auto role = newUserDlg.getRole();
            emit doModifyUser(id, fullname, login, password, role);
        }
    }
}

void CUserManagementWidget::onDeleteUser()
{
    auto selectionModel = m_pUserTable->selectionModel();
    if(selectionModel && selectionModel->hasSelection() == true)
    {
        auto pModel = m_pUserTable->model();
        auto indexes = selectionModel->selectedRows();

        for(int i=0; i<indexes.count(); ++i)
        {
            int id = pModel->data(pModel->index(indexes[i].row(), 0)).toInt();
            emit doDeleteUser(id);
        }
    }
}

void CUserManagementWidget::onTableViewClicked(const QModelIndex &index)
{
    if(index.isValid() == false)
    {
        m_pBtnEdit->setEnabled(false);
        return;
    }

    auto pModel = m_pUserTable->model();
    int id = pModel->data(pModel->index(index.row(), 0)).toInt();
    m_pBtnEdit->setEnabled(id == m_currentUserId || m_currentUserRole == CUserManager::ADMINISTRATOR);
}
