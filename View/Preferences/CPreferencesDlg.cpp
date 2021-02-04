#include "CPreferencesDlg.h"
#include <QStackedWidget>
#include "CUserManagementWidget.h"
#include "CProtocolSettingsWidget.h"
#include "Widgets/FancyTabBar/fancytabbar.h"

CPreferencesDlg::CPreferencesDlg(QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Preferences"), parent, DEFAULT|EFFECT_ENABLED, f)
{
    initLayout();
    initConnections();
}

CGeneralSettingsWidget*CPreferencesDlg::getGeneralSettings() const
{
    return m_pGeneralSettingsWidget;
}

CProtocolSettingsWidget *CPreferencesDlg::getProtocolSettings() const
{
    return m_pProtocolSettingsWidget;
}

void CPreferencesDlg::initLayout()
{
    //m_pUserManagementWidget = new CUserManagementWidget;
    m_pGeneralSettingsWidget = new CGeneralSettingsWidget;
    m_pProtocolSettingsWidget = new CProtocolSettingsWidget;

    m_pTabBar = new FancyTabBar(FancyTabBar::Left);
    m_pTabBar->insertTab(0, QIcon(":/Images/settings-3d.png"), tr("General"));
    m_pTabBar->insertTab(1, QIcon(":/Images/protocols-white.png"), tr("Workflow"));
    //m_pTabBar->insertTab(1, QIcon(":/Images/avatar.png"), tr("Users"));
    m_pTabBar->setTabEnabled(0, true);
    m_pTabBar->setTabEnabled(1, true);

    m_pStackWidget = new QStackedWidget;
    m_pStackWidget->addWidget(m_pGeneralSettingsWidget);
    m_pStackWidget->addWidget(m_pProtocolSettingsWidget);
    //m_pStackWidget->addWidget(m_pUserManagementWidget);
    m_pStackWidget->setCurrentIndex(0);

    auto pHLayout = new QHBoxLayout;
    pHLayout->addWidget(m_pTabBar);
    pHLayout->addWidget(m_pStackWidget);

    auto pVLayout = getContentLayout();
    pVLayout->addLayout(pHLayout);
}

void CPreferencesDlg::initConnections()
{
    connect(m_pTabBar, &FancyTabBar::currentChanged, [&](int index){ m_pStackWidget->setCurrentIndex(index); });
}
