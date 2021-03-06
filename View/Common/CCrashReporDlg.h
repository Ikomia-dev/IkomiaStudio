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

#ifndef CCRASHREPORDLG_H
#define CCRASHREPORDLG_H

#include "CDialog.h"
#include <QNetworkReply>

class CCrashReporDlg : public CDialog
{
    public:
        explicit CCrashReporDlg(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void    setProgressBarMaximum(qint64 totalBytes);

    public slots:
        void    onSendDumpFiles();
        void    onQuit();
        void    onUpdateProgress(qint64 sentBytes, qint64 totalBytes);
        void    onError(const QString& err);

    private:
        void    deleteDumpFiles() const;
        void    init();
        void    initInformation();

    private:
        QLabel*         m_pExplainLabel = nullptr;
        QTextBrowser*   m_pTextBrowser = nullptr;
        QProgressBar*   m_pProgress = nullptr;
};

#endif // CCRASHREPORDLG_H
