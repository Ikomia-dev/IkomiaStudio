#include "CProtocolSettingsWidget.h"
#include "Widgets/CBrowseFileWidget.h"

CProtocolSettingsWidget::CProtocolSettingsWidget(QWidget *parent): QWidget(parent)
{
    initLayout();
    initConnections();
}

void CProtocolSettingsWidget::onSetSaveFolder(const QString &path)
{
    m_pBrowseWidget->setPath(path);
}

void CProtocolSettingsWidget::initLayout()
{
    auto pLabel = new QLabel(tr("Auto-save folder"));

    m_pBrowseWidget = new CBrowseFileWidget();
    m_pBrowseWidget->setMode(QFileDialog::FileMode::Directory);

    auto pLayout = new QGridLayout;
    pLayout->addWidget(pLabel, 0, 0);
    pLayout->addWidget(m_pBrowseWidget, 0, 1);
    setLayout(pLayout);
}

void CProtocolSettingsWidget::initConnections()
{
    connect(m_pBrowseWidget, &CBrowseFileWidget::selectedFileChanged, [&](const QString& path){ emit doSetSaveFolder(path);});
}
