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
            {"AGPL 3.0", "AGPL_30"},
            {"Apache 2.0", "APACHE_20"},
            {"BSD 3-Clause", "BSD_3_CLAUSE"},
            {"Creative Common 1.0", "CC0_10"},
            {"GPL 3.0", "GPL_30"},
            {"LGPL 3.0", "LGPL_30"},
            {"MIT", "MIT"}
        };
};

#endif // CPUBLICATIONFORMDLG_H
