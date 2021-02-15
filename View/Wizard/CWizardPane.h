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
