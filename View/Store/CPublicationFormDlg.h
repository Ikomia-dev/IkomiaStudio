#ifndef CPUBLICATIONFORMDLG_H
#define CPUBLICATIONFORMDLG_H

#include "View/Common/CDialog.h"

class CStoreQueryModel;

class CPublicationFormDlg: public CDialog
{
    Q_OBJECT

    public:

        CPublicationFormDlg(const QSqlRecord& pluginInfo, const QJsonObject &publishInfo, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        QJsonObject getPublishInfo() const;

    private:

        void    initLayout();
        void    initConnections();

        void    fillLicences();
        void    fillVersions();

    private:

        QSqlRecord      m_pluginInfo;
        QJsonObject     m_publishInfo;
        QComboBox*      m_pComboLicense = nullptr;
        QComboBox*      m_pComboVersion = nullptr;
        QPushButton*    m_pOkBtn = nullptr;
        QPushButton*    m_pCancelBtn = nullptr;

        const std::map<QString, QString> m_licenses =
        {
            {"AGPL_30", "AGPL 3.0"},
            {"APACHE_20", "Apache 2.0"},
            {"BSD_3_CLAUSE", "BSD 3-Clause"},
            {"BSD_2_CLAUSE", "BSD 2-Clause"},
            {"CC0_10", "Creative Commons 1.0"},
            {"CC_BY_NC_40", "Creative Commons Non Commercial 4.0"},
            {"GPL_30", "GPL 3.0"},
            {"LGPL_30", "LGPL 3.0"},
            {"MIT", "MIT"},
            {"CUSTOM", "Custom"},
        };
};

#endif // CPUBLICATIONFORMDLG_H
