#include "CPublicationFormDlg.h"
#include "Model/Store/CStoreQueryModel.h"

CPublicationFormDlg::CPublicationFormDlg(const QSqlRecord &pluginInfo, const QJsonObject& publishInfo, QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Ikomia HUB publication "), parent, DEFAULT|EFFECT_ENABLED, f)
{
    m_pluginInfo = pluginInfo;
    m_publishInfo = publishInfo;
    initLayout();
    initConnections();
}

QJsonObject CPublicationFormDlg::getPublishInfo() const
{
    QJsonObject info;
    info["license"] = m_pComboLicense->currentData().toString();
    info["version"] = m_pComboVersion->currentData().toString();
    return info;
}

void CPublicationFormDlg::initLayout()
{
    // Licence
    auto pLabelLicence = new QLabel(tr("Algorithm licence"));
    m_pComboLicense = new QComboBox;
    fillLicences();

    // Version
    auto pLabelVersion = new QLabel(tr("Algorithm version"));
    m_pComboVersion = new QComboBox;
    fillVersions();

    // Layout
    auto pLayout = new QGridLayout();
    pLayout->addWidget(pLabelLicence, 0, 0);
    pLayout->addWidget(m_pComboLicense, 0, 1);
    pLayout->addWidget(pLabelVersion, 1, 0);
    pLayout->addWidget(m_pComboVersion, 1, 1);

    // OK - Cancel buttons
    m_pOkBtn = new QPushButton(tr("OK"));
    m_pOkBtn->setDefault(true);
    m_pCancelBtn = new QPushButton(tr("Cancel"));

    QHBoxLayout* pBtnLayout = new QHBoxLayout;
    pBtnLayout->addWidget(m_pOkBtn);
    pBtnLayout->addWidget(m_pCancelBtn);

    auto pMainLayout = getContentLayout();
    pMainLayout->addSpacing(5);
    pMainLayout->addLayout(pLayout);
    pMainLayout->addSpacing(5);
    pMainLayout->addLayout(pBtnLayout);
}

void CPublicationFormDlg::initConnections()
{
    connect(m_pOkBtn, &QPushButton::clicked, this, &CPublicationFormDlg::accept);
    connect(m_pCancelBtn, &QPushButton::clicked, this, &CPublicationFormDlg::reject);
}

void CPublicationFormDlg::fillLicences()
{
    for (const auto& l : _officialLicenses)
        m_pComboLicense->addItem(l.second, l.first);

    auto currentLicense = m_pluginInfo.value("license").toString();
    if (currentLicense.isEmpty() == false)
    {
        auto it = _officialLicenses.find(currentLicense);
        if (it == _officialLicenses.end())
            m_pComboLicense->addItem(currentLicense, currentLicense);

        m_pComboLicense->setCurrentIndex(m_pComboLicense->findData(currentLicense));
    }
    else
        m_pComboLicense->setCurrentIndex(m_pComboLicense->findData(m_publishInfo["license"].toString()));
}

void CPublicationFormDlg::fillVersions()
{
    auto currentVersion = m_pluginInfo.value("version").toString();
    if (currentVersion.isEmpty() == false)
        m_pComboVersion->addItem("Source version: " + currentVersion, currentVersion);

    QJsonObject versions = m_publishInfo["next_versions"].toObject();
    auto major = versions["major"].toString();
    m_pComboVersion->addItem("Next major: " + major, major);
    auto minor = versions["minor"].toString();
    m_pComboVersion->addItem("Next minor: " + minor, minor);
    auto patch = versions["patch"].toString();
    m_pComboVersion->addItem("Next patch: " + patch, patch);

    if (currentVersion.isEmpty() == false)
        m_pComboVersion->setCurrentIndex(m_pComboVersion->findData(currentVersion));
    else
        m_pComboVersion->setCurrentIndex(m_pComboVersion->findData(minor));
}
