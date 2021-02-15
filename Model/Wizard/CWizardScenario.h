/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CWIZARDSCENARIO_H
#define CWIZARDSCENARIO_H

#include "Model/CMainModel.h"
#include "View/Main/CMainView.h"
#include <QList>

struct WizardStep
{
        QWidget* pWidget;
        QString text;
        QString description;
        QObject* pSender;
        const char* pSignal;
        const char* pSlot;
        QString styleSheet = "border: 2px solid green; border-radius: 5px;";
        int flags = 0;
};

class CWizardScenarioInfo
{
    public:

        CWizardScenarioInfo() {}

        std::string         getName() const { return m_name; }
        std::string         getDescription() const { return m_description; }
        std::string         getAuthors() const { return m_authors; }
        std::string         getKeywords() const { return m_keywords; }
        std::string         getIconPath() const { return m_iconPath; }

    public:

        std::string         m_name;
        std::string         m_description;
        std::string         m_authors;
        std::string         m_keywords;
        std::string         m_iconPath;
};

class CWizardScenario : public QObject
{
        Q_OBJECT
    public:
        CWizardScenario(CMainModel* pModel, CMainView* pView);

        QList<WizardStep>   getScenario() const;

    signals:
        void                doNext();

    protected:
        virtual void        makeScenario() = 0;

        void                addStep(QWidget* pWidget, const QString& text, const QString& description, const QString& styleSheet, QObject* pSender, const char* pSignal, const char* pSlot = nullptr, int flags = 0);

    protected:
        QList<WizardStep>   m_stepList;
        CMainModel*         m_pModel = nullptr;
        CMainView*          m_pView = nullptr;
};

//---------------------------//
//----- Tutorial factory -----//
//---------------------------//

class CWizardScenarioFactory
{
    public:

        virtual ~CWizardScenarioFactory() {}

        CWizardScenarioInfo&        getInfo() { return m_info; }
        CWizardScenarioInfo         getInfo() const { return m_info; }

        void                        setInfo(const CWizardScenarioInfo& info) { m_info = info; }

        virtual CWizardScenario*    create(CMainModel* pModel, CMainView* pView) = 0;

    protected:

        CWizardScenarioInfo         m_info;
};

using TutorialFactoryPtr = std::shared_ptr<CWizardScenarioFactory>;
using TutorialFactories = std::vector<TutorialFactoryPtr>;

//-------------------------------------//
//----- Process abstract factory -----//
//-----------------------------------//
class CTutorialAbstractFactory : public CAbstractFactory<std::string, CWizardScenario*>
{
    public:

        TutorialFactories& getList()
        {
            return m_factories;
        }

    private:

        TutorialFactories m_factories;
};

#endif // CWIZARDSCENARIO_H
