#ifndef CWIZARDSTEPMODEL_H
#define CWIZARDSTEPMODEL_H

#include <QStandardItemModel>

struct WizardStep;

class CWizardItem : public QStandardItem
{
    public:

        using QStandardItem::QStandardItem;

        int type() const
        {
            return m_type;
        }

        void setType(int type)
        {
            m_type = type;
        }

    private:
        int m_type = QStandardItem::Type;
};

class CWizardStepModel : public QStandardItemModel
{
    Q_OBJECT

    public:

        CWizardStepModel(const QList<WizardStep> stepList, QObject* parent = nullptr);

};

#endif // CWIZARDSTEPMODEL_H
