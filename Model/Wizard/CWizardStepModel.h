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
