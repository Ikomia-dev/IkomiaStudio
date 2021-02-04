#ifndef CPROCESSPARAMETERS_H
#define CPROCESSPARAMETERS_H

#include "View/Common/CDialog.h"
#include "Main/forwards.hpp"
#include "Core/CProcessInfo.h"
#include "Model/User/CUser.h"

class CResizeStackedWidget;
class CProcessDocDlg;

class CProcessParameters : public CDialog
{
    Q_OBJECT

    public:

        CProcessParameters(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void            addWidget(QWidget* pWidget, QString processName);

        void            setCurrentWidget(const QString& processName);
        void            setProcessInfo(const CProcessInfo& info);
        void            setCurrentUser(const CUser& user);

        QWidget*        getWidget();

        bool            isCurrent(QString name) const;

        void            fitToContent();

        void            clear();

        void            remove(const QString& name);

    signals:

        void            doUpdateProcessInfo(bool bFullEdit, const CProcessInfo& info);

    protected:

        void            hideEvent(QHideEvent* event) override;

    private slots:

        void            onShowDocumentation();

    private:

        void            initLayout();
        void            initConnections();

        QFrame*         createLine();

        CProcessInfo    getProcessInfo() const;

        void            updateWidgetPosition(QWidget* pWidget, int borderSize);

    private:

        CResizeStackedWidget*   m_pParamWidgets = nullptr;
        QLabel*                 m_pLabel = nullptr;
        QPushButton*            m_pBtnDocumentation = nullptr;
        QList<QString>          m_widgetNames;
        CProcessDocDlg*         m_pDocDlg = nullptr;
};

#endif // CPROCESSPARAMETERS_H
