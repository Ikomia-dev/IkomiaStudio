#include "CWorkflowPackage.h"
#include "Main/AppTools.hpp"
#include "JlCompress.h"

CWorkflowPackage::CWorkflowPackage(const QString &wfPath)
{
    if (Utils::File::isFileExist(wfPath.toStdString()) == false)
    {
        std::string error = "Package creation failed, file " + wfPath.toStdString() + " does not exist";
        throw CException(CoreExCode::INVALID_FILE, error, __FILE__, __func__, __LINE__);
    }
    m_path = wfPath;

    QFile jsonFile(wfPath);
    if(!jsonFile.open(QFile::ReadOnly))
        throw CException(CoreExCode::INVALID_FILE, "Package creation failed, could not load JSON file: " + wfPath.toStdString(), __func__, __FILE__, __LINE__);

    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonFile.readAll()));
    if(jsonDoc.isNull() || jsonDoc.isEmpty())
        throw CException(CoreExCode::INVALID_JSON_FORMAT, "Package creation failed, invalid JSON structure", __func__, __FILE__, __LINE__);

    m_workflow = jsonDoc.object();
}

CWorkflowPackage::~CWorkflowPackage()
{
    // Cleanup
    boost::filesystem::remove_all(m_packageDir.toStdString());
}

QString CWorkflowPackage::create()
{
    // Create working directory
    m_packageDir = QString("%1/Workflows/ScalePackage").arg(Utils::IkomiaApp::getQIkomiaFolder());
    Utils::File::createDirectory(m_packageDir.toStdString());

    // Add plugins
    addAlgorithms();

    // Save workflow file
    QString newPath = QString("%1/workflow.json").arg(m_packageDir);
    QFile jsonFile(newPath);
    if(!jsonFile.open(QFile::WriteOnly))
        throw CException(CoreExCode::INVALID_FILE, "Package creation failed, could not save workflow: " + m_path.toStdString(), __func__, __FILE__, __LINE__);

    QJsonDocument jsonDoc(m_workflow);
    jsonFile.write(jsonDoc.toJson());
    jsonFile.close();

    // Zip folder
    QString zipPath = QString("%1/Workflows/archive.zip").arg(Utils::IkomiaApp::getQIkomiaFolder());
    JlCompress::compressDir(zipPath, m_packageDir, true);
    return zipPath;
}

void CWorkflowPackage::addAlgorithms()
{
    bool bUpdate = false;
    QJsonArray algos = m_workflow["tasks"].toArray();

    for (int i=0; i<algos.size(); ++i)
    {
        QJsonObject algo = algos[i].toObject();
        QJsonObject data = algo["task_data"].toObject();

        if (data.contains("url"))
        {
            QString algoDirectory = data["url"].toString();
            algoDirectory.remove("file://");
            QString targetDirectory = QString("%1/%2").arg(m_packageDir).arg(data["name"].toString());
            Utils::File::copyDirectory(algoDirectory, targetDirectory, true);

            if (updatePathInParameters(data))
            {
                algo["task_data"] = data;
                bUpdate = true;
            }
        }

        if (bUpdate)
            algos[i] = algo;
    }

    if (bUpdate)
        m_workflow["tasks"] = algos;
}

bool CWorkflowPackage::updatePathInParameters(QJsonObject& data)
{
    bool bUpdateData = false;
    QString algoDirectory = data["url"].toString();
    algoDirectory.remove("file://");
    QString targetAlgoDir = QString("%1/%2").arg(m_packageDir).arg(data["name"].toString());
    QJsonArray parameters = data["parameters"].toArray();
    // Seach for Windows or Linux path
    QRegularExpression re("(^(([a-zA-Z]{1}:{1}\\\{1,2})|(\\/{1}))[\\w]{1}.*)");

    for (int i=0; i<parameters.size(); ++i)
    {
        bool bUpdateParam = false;
        QJsonObject param = parameters[i].toObject();
        QString value = param["value"].toString();
        QRegularExpressionMatch match = re.match(value);

        if(match.hasMatch())
        {
            QString path = match.captured(0);
            if (Utils::File::isFileExist(path.toStdString()) && path.contains(algoDirectory) == false)
            {
                // File is external, copy it in working directory and update path
                QString dataDir = QString("%1/data").arg(targetAlgoDir);
                Utils::File::createDirectory(dataDir.toStdString());
                QString filename = QString::fromStdString(Utils::File::getFileName(path.toStdString()));
                QString targetPath = QString("%1/%2").arg(dataDir).arg(filename);
                boost::filesystem::copy_file(path.toStdString(), targetPath.toStdString());
                // Set path as relative
                value = QString("file://data/%2").arg(filename);
                bUpdateParam = true;
            }
        }

        if (bUpdateParam)
        {
            param["value"] = value;
            parameters[i] = param;
            bUpdateData = true;
        }
    }

    if (bUpdateData)
        data["parameters"] = parameters;

    return bUpdateData;
}
