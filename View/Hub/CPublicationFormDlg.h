#ifndef CPUBLICATIONFORMDLG_H
#define CPUBLICATIONFORMDLG_H

#include "View/Common/CDialog.h"

class CHubQueryModel;

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
};

#endif // CPUBLICATIONFORMDLG_H
