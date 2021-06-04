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

#ifndef CSETTINGSMANAGER_H
#define CSETTINGSMANAGER_H

#include <QSqlDatabase>
#include <QObject>
#include <QFileDialog>

class CWizardManager;

class CSettingsManager : public QObject
{
    Q_OBJECT

    public:

        CSettingsManager();

        void        init();
        void        notifyViewShow();

        std::string getWorkflowSaveFolder() const;

        bool        isNativeDlgEnabled() const;
        bool        isTutorialEnabled() const;

        void        enableTutorial(bool bEnable);

        static QFileDialog::Options dialogOptions();

    signals:

        void        doEnableNativeDialog(bool bEnable);
        void        doEnableTutorialHelper(bool bEnable);
        void        doSetWorkflowSaveFolder(const QString& path);

    public slots:

        void        onUseNativeDlg(bool bEnable);
        void        onEnableTutorialHelper(bool bEnable);
        void        onSetWorkflowSaveFolder(const QString& path);

    private:

        void        initMainDb();
        void        initNativeDialogOption();
        void        initTutorialHelperOption();
        void        initWorkflowOption();

        void        setSettings(const QString& category, const QJsonObject &jsonData);
        void        setUseNativeDlg(bool bEnable);
        void        setTutoEnabled(bool bEnable);

        static void setDialogOptions(QFileDialog::Options options);

        QJsonObject getSettings(const QString& category) const;

    private:

        static QFileDialog::Options m_dlgOptions;

        QSqlDatabase    m_mainDb;
        bool            m_bUseNativeDlg = false;
        bool            m_bShowTuto = true;
        std::string     m_protocolSaveFolder;
};

#endif // CSETTINGSMANAGER_H
