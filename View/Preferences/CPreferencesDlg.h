#ifndef CPREFERENCESDLG_H
#define CPREFERENCESDLG_H

#include "View/Common/CDialog.h"
#include "CGeneralSettingsWidget.h"

class FancyTabBar;
class CUserManagementWidget;
class QSqlQueryModel;
class CProtocolSettingsWidget;

// Not use for the moment
class CPreferencesDlg : public CDialog
{
    Q_OBJECT

    public:

        CPreferencesDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        CGeneralSettingsWidget*     getGeneralSettings() const;
        CProtocolSettingsWidget*    getProtocolSettings() const;

    private:

        void                        initLayout();
        void                        initConnections();

    private:

        FancyTabBar*                m_pTabBar = nullptr;
        QStackedWidget*             m_pStackWidget = nullptr;
        CUserManagementWidget*      m_pUserManagementWidget = nullptr;
        CGeneralSettingsWidget*     m_pGeneralSettingsWidget = nullptr;
        CProtocolSettingsWidget*    m_pProtocolSettingsWidget = nullptr;
};

#endif // CPREFERENCESDLG_H
