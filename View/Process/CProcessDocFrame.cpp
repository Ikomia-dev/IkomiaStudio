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
#include "Model/Plugin/CPluginTools.h"

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

    m_pBrowser = new QTextBrowser;
    m_pDoc = new QTextDocument;
    m_pBrowser->setOpenExternalLinks(true);
    m_pBrowser->setDocument(m_pDoc);
    pMainLayout->addWidget(m_pBrowser);

    setLayout(pMainLayout);
}

void CProcessDocFrame::fillDocumentation(const CTaskInfo &info)
{
    QString pluginDir;
    QString pluginName = QString::fromStdString(info.m_name);

    if(info.m_language == ApiLanguage::CPP)
        pluginDir = Utils::CPluginTools::getCppPluginFolder(pluginName);
    else
        pluginDir = Utils::CPluginTools::getPythonPluginFolder(pluginName);

    QString docFilePath;
    QDir qpluginDir(pluginDir);

    // Check if local doc file exists
    foreach (QString fileName, qpluginDir.entryList(QDir::Files|QDir::NoSymLinks))
    {
        if(m_docFiles.contains(fileName))
        {
            docFilePath = qpluginDir.absoluteFilePath(fileName);
            break;
        }
    }

    if(!docFilePath.isEmpty())
    {
        // Load doc file
        QFile file(docFilePath);
        if(file.open(QFile::ReadOnly | QFile::Text) == false)
        {
            docFilePath.clear();
            qWarning().noquote() << tr("Found local document file for plugin %1 but loading failed.").arg(pluginName);
        }
        else
        {
            QString mdContent(file.readAll());
            updateLocalPath(mdContent, pluginName);
            m_pDoc->setMarkdown(mdContent);
        }
    }

    if(docFilePath.isEmpty())
    {
        // Generate doc from plugin metadata
        QString mdContent = generateMarkdown(info);
        m_pDoc->setMarkdown(mdContent);
    }
}

QString CProcessDocFrame::getMarkdownTemplate() const
{
    return QString("_icon_ \n\n"
                   "<span style=\"color:#cc5a20\"><h1>_name_</h1></span> ![Language logo](_languageIcon_)  \n\n"
                   "**Version**: _version_  \n"
                   "**_modifiedTxt_**: _modified_  \n"
                   "**_createdTxt_**: _created_  \n"
                   "**_statusLabel_**: _status_  \n"
                   "_description_\n\n"
                   "_docLink_ \n\n"
                   "<span style=\"color:#cc5a20\"><h2>Publication</h2></span> \n\n"
                   "_article_ \n\n"
                   "*_authors_* \n\n"
                   "_journal_ \n\n"
                   "_year_ \n\n"
                   "_repo_ \n\n"
                   "_license_"
                   "<span style=\"color:#cc5a20\"><h2>_keywordsTxt_</h2></span> \n\n"
                   "_keywords_");
}

QString CProcessDocFrame::getStatus(const CTaskInfo &info) const
{
    QString status;
    QString ikomiaVersion = QString::fromStdString(info.m_ikomiaVersion);

    if(info.m_language == ApiLanguage::CPP)
    {
        PluginState state = Utils::Plugin::getCppState(ikomiaVersion);
        if(state == PluginState::DEPRECATED)
        {
            status = tr("<span style=\"color:#9a0000\">deprecated</span>(based on Ikomia version %1 while the current is %2)")
                    .arg(ikomiaVersion)
                    .arg(Utils::IkomiaApp::getCurrentVersionNumber());
        }
        else if(state == PluginState::UPDATED)
        {
            status = tr("<span style=\"color:#9a0000\">Ikomia Studio update required</span>(based on Ikomia version %1 while the current is %2)")
                    .arg(ikomiaVersion)
                    .arg(Utils::IkomiaApp::getCurrentVersionNumber());
        }
        else
        {
            status = tr("<span style=\"color:#008f00\">OK</span> (based on Ikomia version %1)").arg(ikomiaVersion);
        }
    }
    else
    {
        PluginState state = Utils::Plugin::getPythonState(ikomiaVersion);
        if(state == PluginState::DEPRECATED)
        {
            status = tr("<span style=\"color:#9a0000\">deprecated</span>(based on Ikomia version %1 while the current is %2)")
                    .arg(ikomiaVersion)
                    .arg(Utils::IkomiaApp::getCurrentVersionNumber());
        }
        else if(state == PluginState::UPDATED)
        {
            status = tr("<span style=\"color:#de7207\">Ikomia Studio update adviced</span>(based on Ikomia version %1 while the current is %2)")
                    .arg(ikomiaVersion)
                    .arg(Utils::IkomiaApp::getCurrentVersionNumber());
        }
        else
        {
            status = tr("<span style=\"color:#008f00\">OK</span> (based on Ikomia version %1)").arg(ikomiaVersion);
        }
    }
    return status;
}

QString CProcessDocFrame::generateMarkdown(const CTaskInfo &info) const
{
    auto templateContent = getMarkdownTemplate();

    QString languageIconPath;
    if(info.m_language == ApiLanguage::PYTHON)
        languageIconPath = "qrc:/Images/python-language-logo-32.png";
    else
        languageIconPath = "qrc:/Images/C++-language-logo-32.png";

    //Process name
    auto newContent = templateContent.replace("_name_", QString::fromStdString(info.m_name));

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

    //Status: deprecated or not
    auto status = getStatus(info);
    newContent = newContent.replace("_statusLabel_", tr("Status"));
    newContent = newContent.replace("_status_", status);

    //Description
    newContent = newContent.replace("_description_", QString::fromStdString(info.m_description));

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

    if(info.m_repo.empty() == false)
    {
        auto repoStr = QString("<span style=\"color:#cc5a20\"><h2>Repository</h2></span> \n\n [%1](%1) \n\n").arg(QString::fromStdString(info.m_repo));
        newContent = newContent.replace("_repo_", repoStr);
    }
    else
        newContent = newContent.replace("_repo_", "");

    if(info.m_license.empty() == false)
    {
        auto licenceStr = QString("<span style=\"color:#cc5a20\"><h2>License</h2></span> \n\n %1 \n\n").arg(QString::fromStdString(info.m_license));
        newContent = newContent.replace("_license_", licenceStr);
    }
    else
        newContent = newContent.replace("_license_", "");

    //Keywords
    QString keywordsTxt = tr("Keywords");
    newContent = newContent.replace("_keywordsTxt_", keywordsTxt);
    newContent = newContent.replace("_keywords_", QString::fromStdString(info.m_keywords));

    return newContent;
}

void CProcessDocFrame::updateLocalPath(QString &content, const QString& name)
{
    QString reStr = QString("file:(.*\\/Ikomia)\\/Plugins\\/.*\\/%1\\/.*\\.[a-zA-Z0-9]*").arg(name);
    QRegularExpression re(reStr);
    QRegularExpressionMatchIterator matchIt = re.globalMatch(content);

    while(matchIt.hasNext())
    {
        QRegularExpressionMatch match = matchIt.next();
        auto fullMatch = match.captured(0);
        auto homeDir = match.captured(1);
        fullMatch.replace(homeDir, Utils::IkomiaApp::getQIkomiaFolder());
        content.replace(match.captured(0), fullMatch);
    }
}
