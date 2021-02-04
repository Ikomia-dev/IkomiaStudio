#include "CGeneralSettingsWidget.h"
#include <QCheckBox>
#include <QVBoxLayout>

CGeneralSettingsWidget::CGeneralSettingsWidget(QWidget* parent) : QWidget(parent)
{
    initLayout();
    initConnections();
}

void CGeneralSettingsWidget::onEnableTutorialHelper(bool bEnable)
{
    // Tutorials standby
    m_pCheckTuto->setChecked(bEnable);
}

void CGeneralSettingsWidget::onEnableNativeDialog(bool bEnable)
{
    m_pCheckNative->setChecked(bEnable);
}

void CGeneralSettingsWidget::initLayout()
{
    QVBoxLayout* pVBoxLayout = new QVBoxLayout;
    // Tutorials standby
    m_pCheckTuto = new QCheckBox(tr("Enable tutorial helper"));
    pVBoxLayout->addWidget(m_pCheckTuto);

    m_pCheckNative = new QCheckBox(tr("Enable native file manager"));
    pVBoxLayout->addWidget(m_pCheckNative);

    setLayout(pVBoxLayout);
}

void CGeneralSettingsWidget::initConnections()
{
    // Tutorials standby
    connect(m_pCheckTuto, &QCheckBox::toggled, [&](bool bEnable){ emit doEnableTutorialHelper(bEnable); });
    connect(m_pCheckNative, &QCheckBox::toggled, [&](bool bEnable){ emit doEnableNativeDialog(bEnable); });
}
