#ifndef CWIZARDDOCPAGE_H
#define CWIZARDDOCPAGE_H

#include "Model/User/CUser.h"
#include <QStackedWidget>

class CWizardDocFrame;
class CWizardScenarioInfo;

class CWizardDocPage : public QWidget
{
        Q_OBJECT

        public:

            enum Action
            {
                NONE = 0x00000000,
                BACK = 0x00000001,
                EDIT = 0x00000002
            };

            CWizardDocPage(int actions, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

            void            setCurrentUser(const CUser& user);
            //void            setTutoInfo(const CWizardScenarioInfo &info);

        signals:

            void            doBack();
            //void            doSave(bool bFullEdit, const CWizardScenarioInfo& info);

        protected:

            virtual void    showEvent(QShowEvent* event) override;

        private:

            void            initLayout();
            void            initConnections();

        private:

            int                    m_actions = EDIT;
            QStackedWidget*        m_pStackWidget = nullptr;
            CWizardDocFrame*       m_pDocFrame = nullptr;
};

#endif // CWIZARDDOCPAGE_H
