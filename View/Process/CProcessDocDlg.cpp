#include "CProcessDocDlg.h"
#include "CProcessDocWidget.h"

CProcessDocDlg::CProcessDocDlg(QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Documentation"), parent, NO_TITLE_BAR, f)
{
    initLayout();
    resize(QSize(500, 500));
}

CProcessDocDlg::CProcessDocDlg(int style, QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Documentation"), parent, style, f)
{
    initLayout();
    resize(QSize(500, 500));
}

CProcessDocDlg::~CProcessDocDlg()
{
}

void CProcessDocDlg::setCurrentUser(const CUser &user)
{
    if(m_pDocWidget)
        m_pDocWidget->setCurrentUser(user);
}

void CProcessDocDlg::setProcessInfo(const CProcessInfo &info)
{
    if(m_pDocWidget)
        m_pDocWidget->setProcessInfo(info);
}

CProcessDocWidget *CProcessDocDlg::getDocWidget()
{
    return m_pDocWidget;
}

void CProcessDocDlg::initLayout()
{
    m_pDocWidget = new CProcessDocWidget(CProcessDocWidget::EDIT);
    auto pMainLayout = getContentLayout();
    pMainLayout->addWidget(m_pDocWidget);
}
