#ifndef CWIZARDPANE_H
#define CWIZARDPANE_H

#include <QWidget>
#include <QStackedWidget>
#include "CWizardTutoListView.h"
#include "CWizardDocPage.h"
#include "CWizardStepPage.h"

class CWizardQueryModel;
class CWizardStepModel;

class CWizardPane : public QWidget
{
        Q_OBJECT
    public:
        explicit CWizardPane(QWidget *parent = nullptr);

        CWizardTutoListView*    getTutoListView();
        CWizardStepPage*        getWizardStepPage();
        CWizardDocPage*         getWizardDocPage();

    signals:

    public slots:

        void                onSetTutorialModel(CWizardQueryModel* pModel);
        void                onSetStepModel(CWizardStepModel* pModel);
        void                onSetTotalSteps(int totalSteps);
        void                onShowStepPage(const QString& stepName, int currentStep, const QString& description);
        void                onShowDocPage(const QModelIndex& index);
        void                onShowMainPage();
        void                onFinish();

    private:

        void                init();
        void                initLayout();
        void                initConnections();

        void                createMainPage();
        void                createStepPage();
        void                createDocPage();

    private:

        QStackedWidget*         m_pStack = nullptr;
        CWizardTutoListView*    m_pListView = nullptr;
        QFrame*                 m_pMainPage = nullptr;
        CWizardStepPage*        m_pStepPage = nullptr;
        CWizardDocPage*         m_pDocPage = nullptr;
        int                     m_stepPageIndex;
        int                     m_docPageIndex;
        int                     m_mainPageIndex;
};

#endif // CWIZARDDLG_H
