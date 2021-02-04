#include "CProtocolInfoDlg.h"

CProtocolInfoDlg::CProtocolInfoDlg(QWidget *parent, Qt::WindowFlags f)
    : CDialog(parent, NO_TITLE_BAR, f | Qt::FramelessWindowHint)
{
    setSizeGripEnabled(false);
    setWindowOpacity(0.9);
    initLayout();
    initConnections();
}

void CProtocolInfoDlg::setName(const QString &name)
{
    m_pLabel->setText(name);
}

void CProtocolInfoDlg::setDescription(const QString &text)
{
    m_pLabelDescription->setText(text);
}

QString CProtocolInfoDlg::getName() const
{
    return m_pLabel->text();
}

void CProtocolInfoDlg::initLayout()
{
    //Label with line underneath
    m_pLabel = new QLabel(tr("Process parameters"));
    m_pLabel->setAlignment(Qt::AlignCenter);
    auto pBottomLine = createLine();

    //Description
    m_pLabelDescription = new QLabel("");

    //Load button
    m_pLoadBtn = new QPushButton(tr("Load"));

    auto pLayout = getContentLayout();
    pLayout->addWidget(m_pLabel);
    pLayout->addWidget(pBottomLine);
    pLayout->addWidget(m_pLabelDescription);
    pLayout->addWidget(m_pLoadBtn);
}

void CProtocolInfoDlg::initConnections()
{
    connect(m_pLoadBtn, &QPushButton::clicked, [&]{ emit doLoadProtocol(); });
}

QFrame *CProtocolInfoDlg::createLine()
{
    auto pLine = new QFrame;
    pLine->setLineWidth(1);
    pLine->setFrameStyle(QFrame::Plain);
    pLine->setFrameShape(QFrame::HLine);
    auto pal = qApp->palette();
    pLine->setStyleSheet(QString("color:%1;").arg(pal.highlight().color().name()));
    return pLine;
}

#include "moc_CProtocolInfoDlg.cpp"
