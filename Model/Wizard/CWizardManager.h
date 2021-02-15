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

#ifndef CWIZARDMANAGER_H
#define CWIZARDMANAGER_H

#include "Model/CMainModel.h"
#include "View/Main/CMainView.h"
#include "CWizardScenario.h"
#include "CWizardDbManager.h"
#include "CWizardQueryModel.h"
#include "CWizardStepModel.h"

class CBubbleTip;

class CWizardManager : public QObject
{
        Q_OBJECT
    public:
        CWizardManager();

        void            setModelView(CMainModel* pModel, CMainView* pView);

        void            launchTutorialHelper();
        void            launchFirstStepHelper();
        void            makeTutorial();
        void            initTutorials();

        void            setTutoEnabled(bool bEnable);

    signals:

        void            doSetTutorialModel(CWizardQueryModel* pModel);
        void            doSetStepModel(CWizardStepModel* pModel);
        void            doSetTotalSteps(int totalSteps);
        void            doUpdateStepPage(const QString& title, int currentStep, const QString& description);
        void            doFinish();

    public slots:

        void            onNext();
        void            onCancel();
        void            onPlayTuto(const QModelIndex& index);

    private:

        void            initFactory();
        void            registerFactory(const TutorialFactoryPtr& factory);

        CBubbleTip*     createBubble(int flags);

        void            playTuto(const QString& name);

        void            showBubble(QWidget* pWidget, const QString& title, const QString& text, const QString& stylesheet, int flags = 0);
        void            showBubble(CBubbleTip* pBubbleTip, QWidget* pWidget, const QString& title, const QString& text, const QString& stylesheet);

        void            createWizardQueryModel();

        void            updateItemIcon();
        void            updateItemState();

        void            reset();
        void            run(WizardStep step);

    private:

        CMainModel*                 m_pModel = nullptr;
        CMainView*                  m_pView = nullptr;
        QWidget*                    m_pCurrentWidget = nullptr;
        CBubbleTip*                 m_pBubbleTip = nullptr;
        CWizardScenario*            m_pScenario = nullptr;
        CWizardQueryModel*          m_pWizardModel = nullptr;
        CWizardStepModel*           m_pStepModel = nullptr;
        QMetaObject::Connection*    m_pSingleShotConnection = nullptr;
        CTutorialAbstractFactory    m_tutorialFactory;
        CWizardDbManager            m_db;
        QList<WizardStep>           m_stepList;
        QString                     m_originalStyleSheet;
        int                         m_nextId = 0;
        int                         m_tutoId = 0;
        bool                        m_bTuto = true;
};

#endif // CWIZARDMANAGER_H
