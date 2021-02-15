// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
