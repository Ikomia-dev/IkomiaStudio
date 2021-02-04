#ifndef CWIZARDSTEPLISTVIEW_H
#define CWIZARDSTEPLISTVIEW_H

#include <QListView>

class CWizardStepListViewDelegate;

class CWizardStepListView : public QListView
{
    Q_OBJECT

    public:
        CWizardStepListView(QWidget* parent = nullptr);

    private:

        CWizardStepListViewDelegate*    m_pDelegate = nullptr;
};

#endif // CWIZARDSTEPLISTVIEW_H
