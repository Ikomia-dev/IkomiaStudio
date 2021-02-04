#ifndef CPROCESSDOCDLG_H
#define CPROCESSDOCDLG_H

#include "View/Common/CDialog.h"
#include "Core/CProcessInfo.h"
#include "Model/User/CUser.h"

class CProcessDocWidget;

class CProcessDocDlg : public CDialog
{
    public:

        CProcessDocDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
        CProcessDocDlg(int style = DEFAULT_FIXED, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
        ~CProcessDocDlg();

        void                setCurrentUser(const CUser& user);
        void                setProcessInfo(const CProcessInfo& info);

        CProcessDocWidget*  getDocWidget();

    private:

        void                initLayout();

    private:

        CProcessDocWidget*   m_pDocWidget = nullptr;
};

#endif // CPROCESSDOCDLG_H
