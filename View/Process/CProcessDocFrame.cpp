// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CProcessDocFrame.h"
#include <QTextDocumentWriter>
#include "UtilsTools.hpp"
#include "Core/CPluginTools.h"

CProcessDocFrame::CProcessDocFrame(QWidget *parent, Qt::WindowFlags f):
    QFrame(parent, f)
{
    initLayout();
}

void CProcessDocFrame::setProcessInfo(const CTaskInfo &info)
{
    fillDocumentation(info);
}

void CProcessDocFrame::saveContent(const QString &path)
{
    QByteArray format = "HTML";
    std::string ext = Utils::File::extension(path.toStdString());

    if(ext == ".md")
        format = "markdown";

    QTextDocumentWriter writer(path, format);
    if(writer.write(m_pDoc))
        qInfo().noquote() << tr("Documentation successfully saved at %1").arg(path);
    else
        qCritical().noquote() << tr("Failed to save documentation at %1").arg(path);
}

void CProcessDocFrame::initLayout()
{
    setProperty("class", "CProcessDocFrame");

    QHBoxLayout* pMainLayout = new QHBoxLayout;
    pMainLayout->setContentsMargins(0, 0, 0, 0);

    m_pBrowserHeader = new QTextBrowser;
    m_pHeaderDoc = new QTextDocument;
    m_pBrowserHeader->setOpenExternalLinks(true);
    m_pBrowserHeader->setDocument(m_pHeaderDoc);

    m_pBrowser = new QTextBrowser;
    m_pDoc = new QTextDocument;
    m_pBrowser->setOpenExternalLinks(true);
    m_pBrowser->setDocument(m_pDoc);

    pMainLayout->addWidget(m_pBrowserHeader);
    pMainLayout->addWidget(m_pBrowser);

    setLayout(pMainLayout);
}

void CProcessDocFrame::fillDocumentation(const CTaskInfo &info)
{
    // Generate header doc from plugin metadata
    QString qmdContent = generateMarkdown(info);
    m_pHeaderDoc->setMarkdown(qmdContent);

    std::string mdContent = Utils::CPluginTools::getDescription(info.m_name);
    if (!mdContent.empty())
    {
        // Remove HTML part
        QRegularExpression re("<(?s).+</.+>");
        QString qmdContent = QString::fromStdString(mdContent);
        qmdContent.replace(re, "");
        // Remove Github emojis
        qmdContent.replace(":rocket:", "");
        qmdContent.replace(":sunny:", "");
        qmdContent.replace(":pencil:", "");
        qmdContent.replace(":mag:", "");
        m_pDoc->setMarkdown(qmdContent, QTextDocument::MarkdownFeature::MarkdownDialectGitHub);
    }
    else
        m_pDoc->clear();
}

QString CProcessDocFrame::getMarkdownTemplate() const
{
    return QString("_icon_ \n\n"
                   "<span style=\"color:#cc5a20\"><h1>_name_</h1></span> ![Language logo](_languageIcon_)  \n\n"
                   "_shortDescription_  \n"
                   "**Version**: _version_  \n"
                   "**_modifiedTxt_**: _modified_  \n"
                   "**_createdTxt_**: _created_  \n"
                   "_docLink_ \n\n"
                   "<span style=\"color:#cc5a20\"><h2>Publication</h2></span> \n\n"
                   "_article_ \n\n"
                   "*_authors_* \n\n"
                   "_journal_ \n\n"
                   "_year_ \n\n"
                   "<span style=\"color:#cc5a20\"><h2>_sourceCodeTxt_</h2></span> \n\n"
                   "**_repoTxt_**: _repo_ \n\n"
                   "**_originalRepoTxt_**: _originalRepo_ \n\n"
                   "**_licenseTxt**: _license_ \n\n"
                   "<span style=\"color:#cc5a20\"><h2>_keywordsTxt_</h2></span> \n\n"
                   "_keywords_");
}

QString CProcessDocFrame::generateMarkdown(const CTaskInfo &info) const
{
    auto templateContent = getMarkdownTemplate();

    QString languageIconPath;
    if(info.m_language == ApiLanguage::PYTHON)
        languageIconPath = ":/Images/python-language-logo-32.png";
    else
        languageIconPath = ":/Images/C++-language-logo-32.png";

    //Process name
    auto newContent = templateContent.replace("_name_", QString::fromStdString(info.m_name));

    //Description
    newContent = newContent.replace("_shortDescription_", QString::fromStdString(info.m_shortDescription));

    //Language icon
    newContent = newContent.replace("_languageIcon_", languageIconPath);

    //Process icon
    if(info.m_iconPath.empty() == false && info.m_iconPath.at(0) != ':')
        newContent = newContent.replace("_icon_", QString("![Process icon >](file:%1)").arg(QString::fromStdString(info.m_iconPath)));
    else
        newContent = newContent.replace("_icon_", "");

    //Version
    newContent = newContent.replace("_version_", QString::fromStdString(info.m_version));

    //Modified date
    QString modifiedTxt = tr("Modified");
    newContent = newContent.replace("_modifiedTxt_", modifiedTxt);
    newContent = newContent.replace("_modified_", QString::fromStdString(info.m_modifiedDate));

    //Created date
    QString createdTxt = tr("Created");
    newContent = newContent.replace("_createdTxt_", createdTxt);
    newContent = newContent.replace("_created_", QString::fromStdString(info.m_createdDate));

    //Documentation link
    if(info.m_docLink.empty() == false)
    {
        QString link = QString("[%1](%2)").arg(tr("Online documentation")).arg(QString::fromStdString(info.m_docLink));
        newContent = newContent.replace("_docLink_", link);
    }
    else
        newContent = newContent.replace("_docLink_", "");

    //Publication
    QString emphaseCode = "";
    if(info.m_article.empty() == false)
        emphaseCode = "**";

    newContent = newContent.replace("_article_", emphaseCode + QString::fromStdString(info.m_article) + emphaseCode);
    newContent = newContent.replace("_journal_", QString::fromStdString(info.m_journal));
    newContent = newContent.replace("_authors_", QString::fromStdString(info.m_authors));

    if(info.m_year > 0)
        newContent = newContent.replace("_year_", QString::number(info.m_year));
    else
        newContent = newContent.replace("_year_", "");

    // Source code
    QString sourceCodeTxt = tr("Source code");
    newContent = newContent.replace("_sourceCodeTxt_", sourceCodeTxt);
    QString repoTxt = tr("Repository");
    newContent = newContent.replace("_repoTxt_", repoTxt);
    newContent = newContent.replace("_repo_", QString::fromStdString(info.m_repo));
    QString originalRepoTxt = tr("Original repository");
    newContent = newContent.replace("_originalRepoTxt_", originalRepoTxt);
    newContent = newContent.replace("_originalRepo_", QString::fromStdString(info.m_originalRepo));
    QString licenseTxt = tr("License");
    newContent = newContent.replace("_licenseTxt", licenseTxt);

    if(info.m_license.empty() == false)
    {
        QString licenseName = QString::fromStdString(info.m_license);
        auto it = _officialLicenses.find(licenseName);
        if (it != _officialLicenses.end())
            licenseName = it->second;

        newContent = newContent.replace("_license_", licenseName);
    }

    //Keywords
    QString keywordsTxt = tr("Keywords");
    newContent = newContent.replace("_keywordsTxt_", keywordsTxt);
    newContent = newContent.replace("_keywords_", QString::fromStdString(info.m_keywords));

    return newContent;
}
