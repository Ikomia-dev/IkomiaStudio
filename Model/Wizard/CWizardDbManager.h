#ifndef CWIZARDDBMANAGER_H
#define CWIZARDDBMANAGER_H

#include <QSqlDatabase>

class CWizardScenarioInfo;

class CWizardDbManager
{
    public:

        CWizardDbManager();

        void            initDb();

        QSqlDatabase    getTutorialDatabase() const;
        QString         getAllTutorialsQuery() const;

        void            addTutorial(const CWizardScenarioInfo& pScenarioInfo);

    private:

        void        createTutoDb();

    private:

        QString m_connectionName = "WizardConnection";
        QString m_name = ":memory:";
        QString m_type = "QSQLITE";
};

#endif // CWIZARDDBMANAGER_H
