#include "CWizardScenario.h"

CWizardScenario::CWizardScenario(CMainModel* pModel, CMainView* pView)
{
    m_pModel = pModel;
    m_pView = pView;
}

QList<WizardStep> CWizardScenario::getScenario() const
{
    return m_stepList;
}

void CWizardScenario::addStep(QWidget* pWidget, const QString& text, const QString& description, const QString& styleSheet, QObject* pSender, const char* pSignal, const char* pSlot, int flags)
{
    // Store parameters
    WizardStep step;
    step.pWidget = pWidget;
    step.text = text;
    step.description = description;
    step.pSender = pSender;
    step.pSignal = pSignal;
    step.pSlot = pSlot;
    step.flags = flags;
    if(!styleSheet.isEmpty())
        step.styleSheet = styleSheet;

    // Add parameters to list
    m_stepList.append(step);
}
