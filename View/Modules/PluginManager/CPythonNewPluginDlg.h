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

#ifndef CPYTHONNEWPLUGINWIDGET_H
#define CPYTHONNEWPLUGINWIDGET_H

#include <QWidget>
#include "View/Common/CDialog.h"

class CPythonNewPluginDlg : public CDialog
{
    Q_OBJECT

    public:

        explicit CPythonNewPluginDlg(QWidget *parent = nullptr, Qt::WindowFlags f = 0);

        QString getName() const;

    private slots:

        void    onGeneratePlugin();

    private:

        void    initLayout();
        void    initConnections();

    private:

        QLineEdit*      m_pEditPyName = nullptr;
        QComboBox*      m_pComboPyProcessType = nullptr;
        QComboBox*      m_pComboPyWidgetType = nullptr;
        QComboBox*      m_pComboPyQt = nullptr;
        QPushButton*    m_pBtnGenerate = nullptr;
        QPushButton*    m_pBtnCancel = nullptr;
        QString         m_name;
};

#endif // CPYTHONNEWPLUGINWIDGET_H
