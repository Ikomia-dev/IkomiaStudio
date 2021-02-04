#ifndef CPROCESSDOCWIDGET_H
#define CPROCESSDOCWIDGET_H

#include <QWidget>
#include "Core/CProcessInfo.h"
#include "Model/User/CUser.h"

class QStackedWidget;
class CProcessDocFrame;
class CProcessEditDocFrame;

class CProcessDocWidget : public QWidget
{
    Q_OBJECT

    public:

        enum Action
        {
            NONE = 0x00000000,
            BACK = 0x00000001,
            EDIT = 0x00000002
        };

        CProcessDocWidget(int actions, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void            setCurrentUser(const CUser& user);
        void            setProcessInfo(const CProcessInfo &info);

    signals:

        void            doBack();
        void            doSave(bool bFullEdit, const CProcessInfo& info);

    protected:

        virtual void    showEvent(QShowEvent* event) override;

    private:

        void            initLayout();
        void            initConnections();

    private:

        int                     m_actions = EDIT;
        bool                    m_bInit = false;
        CUser                   m_currentUser;
        QStackedWidget*         m_pStackWidget = nullptr;
        CProcessDocFrame*       m_pDocFrame = nullptr;
        CProcessEditDocFrame*   m_pEditDocFrame = nullptr;
};

#endif // CPROCESSDOCWIDGET_H
