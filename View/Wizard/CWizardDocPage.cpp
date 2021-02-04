#include "CWizardDocPage.h"
#include <QtWidgets>
#include "CWizardDocFrame.h"

CWizardDocPage::CWizardDocPage(int actions, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    m_actions = actions;
    initLayout();
    initConnections();
}

void CWizardDocPage::setCurrentUser(const CUser &user)
{
    Q_UNUSED(user);
    //m_pEditDocFrame->setCurrentUser(user);
}

/*void CWizardDocPage::setTutoInfo(const CWizardScenarioInfo& info)
{
    if(m_pDocFrame)
        m_pDocFrame->setTutoInfo(info);

    if(m_pEditDocFrame)
        m_pEditDocFrame->setProcessInfo(info);
}*/

void CWizardDocPage::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_pStackWidget->setCurrentIndex(0);
}

void CWizardDocPage::initLayout()
{
    QHBoxLayout* pActionsLayout = new QHBoxLayout;
    if(m_actions & BACK)
    {
        QPushButton* pBackBtn = new QPushButton(QIcon(":/Images/back.png"), "");
        pBackBtn->setToolTip(tr("Back to process store"));
        connect(pBackBtn, &QPushButton::clicked, [&]{ emit doBack();});
        pActionsLayout->addWidget(pBackBtn);
    }

    if(m_actions & EDIT)
    {
        QPushButton* pEditBtn = new QPushButton(QIcon(":/Images/edit.png"), "");
        pEditBtn->setToolTip(tr("Edit documentation"));
        connect(pEditBtn, &QPushButton::clicked, [&]{ m_pStackWidget->setCurrentIndex(1); });
        pActionsLayout->addWidget(pEditBtn);
    }
    pActionsLayout->addStretch(1);

    m_pDocFrame = new CWizardDocFrame;
    //m_pEditDocFrame = new CProcessEditDocFrame;

    m_pStackWidget = new QStackedWidget;
    m_pStackWidget->addWidget(m_pDocFrame);
    //m_pStackWidget->addWidget(m_pEditDocFrame);

    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pActionsLayout);
    pMainLayout->addWidget(m_pStackWidget);

    setLayout(pMainLayout);
}

void CWizardDocPage::initConnections()
{
    /*connect(m_pEditDocFrame, &CProcessEditDocFrame::doCancel, [&]{ m_pStackWidget->setCurrentIndex(0); });
    connect(m_pEditDocFrame, &CProcessEditDocFrame::doSave, [&](bool bFullEdit, const CProcessInfo& info)
    {
        emit doSave(bFullEdit, info);
        m_pDocFrame->setProcessInfo(info);
        m_pStackWidget->setCurrentIndex(0);
    });*/
}
