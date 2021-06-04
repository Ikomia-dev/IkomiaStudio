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

#ifndef CPROCESSDOCFRAME_H
#define CPROCESSDOCFRAME_H

#include "CProcessDocContent.h"
#include "Core/CTaskInfo.h"

class QTextBrowser;
class QTextDocument;

class CProcessDocFrame : public QFrame
{
    public:

        CProcessDocFrame(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void    setProcessInfo(const CTaskInfo& info);

        void    saveContent(const QString& path);

    private:

        void    initLayout();

        void    fillDocumentation(const CTaskInfo& info);

        QString getMarkdownTemplate() const;
        QString getStatus(const CTaskInfo &info) const;

        QString generateMarkdown(const CTaskInfo& info) const;

        void    updateLocalPath(QString& content, const QString &name);

    private:

        CProcessDocContent  m_content;
        QTextBrowser*       m_pBrowser = nullptr;
        QTextDocument*      m_pDoc = nullptr;

        // List of file patterns used to search for plugin documentation file
        // readme.md is reserved for git-based repository information
        const QSet<QString> m_docFiles = {
            "doc.md", "doc.html", "doc.htm",
            "documentation.md", "documentation.html", "documentation.htm",
            "info.md", "info.html", "info.htm"
        };
};

#endif // CPROCESSDOCFRAME_H
