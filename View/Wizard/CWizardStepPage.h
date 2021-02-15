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
