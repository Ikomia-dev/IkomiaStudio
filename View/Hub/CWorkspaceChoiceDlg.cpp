#include "CWorkspaceChoiceDlg.h"
#include "Main/AppTools.hpp"

CWorkspaceChoiceDlg::CWorkspaceChoiceDlg(const CUser &user, QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Destination workspace"), parent, DEFAULT|EFFECT_ENABLED, f)
{
    initLayout(user);
    initConnections();
}

QString CWorkspaceChoiceDlg::getWorkspacePath() const
{
    return m_pComboNamespaces->currentData().toString();
}

void CWorkspaceChoiceDlg::initLayout(const CUser& user)
{
    auto pLabel = new QLabel(tr("Available workspaces"));

    m_pComboNamespaces = new QComboBox;
    for (size_t i=0; i<user.getNamespaceCount(); ++i)
    {
        auto ns = user.getNamespace(i);
        m_pComboNamespaces->addItem(Utils::User::getNamespaceDisplayName(ns.m_path), ns.m_path);
    }

    m_pComboNamespaces->view()->setMinimumWidth(100);
    m_pComboNamespaces->setCurrentIndex(0);

    auto pLayout = new QGridLayout();
    pLayout->addWidget(pLabel, 0, 0);
    pLayout->addWidget(m_pComboNamespaces, 0, 1);

    m_pOkBtn = new QPushButton(tr("OK"));
    m_pOkBtn->setDefault(true);
    m_pCancelBtn = new QPushButton(tr("Cancel"));

    QHBoxLayout* pBtnLayout = new QHBoxLayout;
    pBtnLayout->addWidget(m_pOkBtn);
    pBtnLayout->addWidget(m_pCancelBtn);

    auto pMainLayout = getContentLayout();
    pMainLayout->addSpacing(5);
    pMainLayout->addLayout(pLayout);
    pMainLayout->addLayout(pBtnLayout);
}

void CWorkspaceChoiceDlg::initConnections()
{
    connect(m_pOkBtn, &QPushButton::clicked, this, &CWorkspaceChoiceDlg::accept);
    connect(m_pCancelBtn, &QPushButton::clicked, this, &CWorkspaceChoiceDlg::reject);
}
