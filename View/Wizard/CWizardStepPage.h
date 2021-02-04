#ifndef CWIZARDSTEPPAGE_H
#define CWIZARDSTEPPAGE_H

#include <QFrame>

class QLabel;
class CWizardStepListView;
class CWizardStepModel;

class CWizardStepPage : public QFrame
{
    Q_OBJECT

    public:
        CWizardStepPage(QWidget* parent = nullptr);

        void    setTitle(const QString& name);
        void    setTotalSteps(int totalSteps);
        void    setCurrentStep(int step);
        void    setDescription(const QString& description);
        void    setListViewModel(CWizardStepModel* pModel);

    signals:

        void    doCancelTuto();

    public slots:

    private:

        void    init();
        void    initLayout();

        void    createGlobalStepView();
        void    createDetailStepView();

    private:

        QLabel*                 m_pTitle = nullptr;
        QLabel*                 m_pStep = nullptr;
        QLabel*                 m_pDescription = nullptr;
        CWizardStepListView*    m_pListView = nullptr;
        QFrame*                 m_pGlobalStepFrame = nullptr;
        QFrame*                 m_pDetailStepFrame = nullptr;
        int                     m_totalSteps;
};

#endif // CWIZARDSTEPPAGE_H
