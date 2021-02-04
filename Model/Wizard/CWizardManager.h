#ifndef CWIZARDMANAGER_H
#define CWIZARDMANAGER_H

#include "Model/CMainModel.h"
#include "View/Main/CMainView.h"
#include "CWizardScenario.h"
#include "CWizardDbManager.h"
#include "CWizardQueryModel.h"
#include "CWizardStepModel.h"

class CBubbleTip;

class CWizardManager : public QObject
{
        Q_OBJECT
    public:
        CWizardManager();

        void            setModelView(CMainModel* pModel, CMainView* pView);

        void            launchTutorialHelper();
        void            launchFirstStepHelper();
        void            makeTutorial();
        void            initTutorials();

        void            setTutoEnabled(bool bEnable);

    signals:

        void            doSetTutorialModel(CWizardQueryModel* pModel);
        void            doSetStepModel(CWizardStepModel* pModel);
        void            doSetTotalSteps(int totalSteps);
        void            doUpdateStepPage(const QString& title, int currentStep, const QString& description);
        void            doFinish();

    public slots:

        void            onNext();
        void            onCancel();
        void            onPlayTuto(const QModelIndex& index);

    private:

        void            initFactory();
        void            registerFactory(const TutorialFactoryPtr& factory);

        CBubbleTip*     createBubble(int flags);

        void            playTuto(const QString& name);

        void            showBubble(QWidget* pWidget, const QString& title, const QString& text, const QString& stylesheet, int flags = 0);
        void            showBubble(CBubbleTip* pBubbleTip, QWidget* pWidget, const QString& title, const QString& text, const QString& stylesheet);

        void            createWizardQueryModel();

        void            updateItemIcon();
        void            updateItemState();

        void            reset();
        void            run(WizardStep step);

    private:

        CMainModel*                 m_pModel = nullptr;
        CMainView*                  m_pView = nullptr;
        QWidget*                    m_pCurrentWidget = nullptr;
        CBubbleTip*                 m_pBubbleTip = nullptr;
        CWizardScenario*            m_pScenario = nullptr;
        CWizardQueryModel*          m_pWizardModel = nullptr;
        CWizardStepModel*           m_pStepModel = nullptr;
        QMetaObject::Connection*    m_pSingleShotConnection = nullptr;
        CTutorialAbstractFactory    m_tutorialFactory;
        CWizardDbManager            m_db;
        QList<WizardStep>           m_stepList;
        QString                     m_originalStyleSheet;
        int                         m_nextId = 0;
        int                         m_tutoId = 0;
        bool                        m_bTuto = true;
};

#endif // CWIZARDMANAGER_H
