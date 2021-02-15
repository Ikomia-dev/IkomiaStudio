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

#ifndef CWIZARDDOCPAGE_H
#define CWIZARDDOCPAGE_H

#include "Model/User/CUser.h"
#include <QStackedWidget>

class CWizardDocFrame;
class CWizardScenarioInfo;

class CWizardDocPage : public QWidget
{
        Q_OBJECT

        public:

            enum Action
            {
                NONE = 0x00000000,
                BACK = 0x00000001,
                EDIT = 0x00000002
            };

            CWizardDocPage(int actions, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

            void            setCurrentUser(const CUser& user);
            //void            setTutoInfo(const CWizardScenarioInfo &info);

        signals:

            void            doBack();
            //void            doSave(bool bFullEdit, const CWizardScenarioInfo& info);

        protected:

            virtual void    showEvent(QShowEvent* event) override;

        private:

            void            initLayout();
            void            initConnections();

        private:

            int                    m_actions = EDIT;
            QStackedWidget*        m_pStackWidget = nullptr;
            CWizardDocFrame*       m_pDocFrame = nullptr;
};

#endif // CWIZARDDOCPAGE_H
