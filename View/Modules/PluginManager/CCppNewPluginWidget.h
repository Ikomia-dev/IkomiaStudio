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

#ifndef CCPPNEWPLUGINWIDGET_H
#define CCPPNEWPLUGINWIDGET_H

#include <QWidget>

class CCppNewPluginWidget : public QWidget
{
    Q_OBJECT

    public:

        explicit CCppNewPluginWidget(QWidget *parent = nullptr);

    signals:

        void    doPluginCreated(const QString& pluginName);

    private slots:

        void    onBrowseSrcFolder();
        void    onBrowseApiFolder();
        void    onGeneratePlugin();

    private:

        void    initLayout();
        void    initApiFolder();
        void    initConnections();

        QString getSelectedFolder();

    private:

        QLineEdit*          m_pEditCppName = nullptr;
        QLineEdit*          m_pEditCppSrcFolder = nullptr;
        QLineEdit*          m_pEditCppApiFolder = nullptr;
        QPushButton*        m_pBtnBrowseSrcFolder = nullptr;
        QPushButton*        m_pBtnBrowseApiFolder = nullptr;
        QComboBox*          m_pComboCppProcessType = nullptr;
        QComboBox*          m_pComboCppWidgetType = nullptr;
        QPushButton*        m_pBtnGenerate = nullptr;
};

#endif // CCPPNEWPLUGINWIDGET_H
