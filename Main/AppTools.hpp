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

#ifndef APPTOOLS_HPP
#define APPTOOLS_HPP

#include <QDir>
#include "Main/AppDefine.hpp"
#include "Main/CoreTools.hpp"
#include "PythonThread.hpp"
#include "Task/CTaskInfo.h"
#include "Model/Settings/CSettingsManager.h"
#include "CImageDataManager.h"
#include "Data/CDataConversion.h"

namespace Ikomia
{
    namespace Utils
    {
        namespace Workflow
        {
            inline QColor       getPortColor(IODataType dataType)
            {
                QColor color = Qt::black;

                switch(dataType)
                {
                    case IODataType::NONE:
                        color = QColor(0, 0, 0);
                        break;

                    case IODataType::IMAGE:
                    case IODataType::IMAGE_BINARY:
                    case IODataType::IMAGE_LABEL:
                        color = QColor(200, 25, 25);
                        break;

                    case IODataType::DESCRIPTORS:
                        color = QColor(200, 150, 120);
                        break;

                    case IODataType::VOLUME:
                    case IODataType::VOLUME_BINARY:
                    case IODataType::VOLUME_LABEL:
                        color = QColor(240, 240, 0);
                        break;

                    case IODataType::INPUT_GRAPHICS:
                    case IODataType::OUTPUT_GRAPHICS:
                        color = QColor(65, 80, 200);
                        break;

                    case IODataType::BLOB_VALUES:
                        color = QColor(160, 50, 160);
                        break;

                    case IODataType::NUMERIC_VALUES:
                        color = QColor(50, 150, 100);
                        break;

                    case IODataType::VIDEO:
                    case IODataType::VIDEO_BINARY:
                    case IODataType::VIDEO_LABEL:
                        color = QColor(0, 190, 165);
                        break;

                    case IODataType::LIVE_STREAM:
                    case IODataType::LIVE_STREAM_BINARY:
                    case IODataType::LIVE_STREAM_LABEL:
                        color = QColor(225, 75, 45);
                        break;

                    case IODataType::WIDGET:
                        color = QColor(105, 255, 85);
                        break;

                    case IODataType::PROJECT_FOLDER:
                        color = QColor(255,0,170);
                        break;

                    case IODataType::FOLDER_PATH:
                        color = QColor(255,0,170);
                        break;

                    case IODataType::FILE_PATH:
                        color = QColor(180,180,180);
                        break;

                    case IODataType::DNN_DATASET:
                        color = QColor(255,255,255);
                        break;

                    case IODataType::DATA_DICT:
                        color = QColor(128,128,128);
                        break;

                    case IODataType::ARRAY:
                        color = QColor(45,180,255);
                        break;

                    case IODataType::OBJECT_DETECTION:
                        color = QColor(40, 5, 110);
                        break;

                    case IODataType::INSTANCE_SEGMENTATION:
                        color = QColor(60, 25, 130);
                        break;

                    case IODataType::SEMANTIC_SEGMENTATION:
                        color = QColor(80, 45, 150);
                        break;

                    case IODataType::KEYPOINTS:
                        color = QColor(80, 10, 220);
                        break;

                    case IODataType::TEXT:
                        color = QColor(30,130,255);
                        break;
                }
                return color;
            }

            inline bool         isPortClickable(IODataType dataType, bool bInput)
            {
                if(bInput == false)
                    return false;

                bool bClickable = false;
                switch(dataType)
                {
                    case IODataType::NONE:
                    case IODataType::IMAGE:
                    case IODataType::IMAGE_BINARY:
                    case IODataType::IMAGE_LABEL:
                    case IODataType::VOLUME:
                    case IODataType::VOLUME_BINARY:
                    case IODataType::VOLUME_LABEL:
                    case IODataType::BLOB_VALUES:
                    case IODataType::NUMERIC_VALUES:
                    case IODataType::VIDEO:
                    case IODataType::VIDEO_BINARY:
                    case IODataType::VIDEO_LABEL:
                    case IODataType::LIVE_STREAM:
                    case IODataType::LIVE_STREAM_BINARY:
                    case IODataType::LIVE_STREAM_LABEL:
                    case IODataType::WIDGET:
                    case IODataType::DESCRIPTORS:
                    case IODataType::OUTPUT_GRAPHICS:
                    case IODataType::PROJECT_FOLDER:
                    case IODataType::DNN_DATASET:
                    case IODataType::ARRAY:
                    case IODataType::DATA_DICT:
                    case IODataType::OBJECT_DETECTION:
                    case IODataType::INSTANCE_SEGMENTATION:
                    case IODataType::SEMANTIC_SEGMENTATION:
                    case IODataType::KEYPOINTS:
                    case IODataType::TEXT:
                        bClickable = false;
                        break;

                    case IODataType::INPUT_GRAPHICS:
                    case IODataType::FOLDER_PATH:
                    case IODataType::FILE_PATH:
                        bClickable = true;
                        break;
                }
                return bClickable;
            }
        }

        namespace Python
        {
            using namespace boost::python;

            inline std::string  getPythonVersion()
            {
                QString pythonPath = Utils::IkomiaApp::getQIkomiaFolder() + "/Python";
                QDir pythonDir(pythonPath);
                if(pythonDir.exists())
                    return "3.10";
                else
                {
                    QSettings ikomiaSettings;
                    const QString PYTHON_BIN_VERSION("python_dev_version");
                    return ikomiaSettings.value(PYTHON_BIN_VERSION, "").toString().toStdString();
                }
            }
            inline std::string  getPythonEnvPath()
            {
                QString pythonPath = Utils::IkomiaApp::getQIkomiaFolder() + "/Python";
                QDir pythonDir(pythonPath);
                if(pythonDir.exists())
                    return pythonPath.toStdString();
                else
                {
                    QSettings ikomiaSettings;
                    const QString PYTHON_INTERPRETER_PATH("python_dev_env_path");
                    return ikomiaSettings.value(PYTHON_INTERPRETER_PATH, "").toString().toStdString();
                }
            }
            inline std::string  getPythonInterpreterPath()
            {
#if defined(Q_OS_LINUX)
                return getPythonEnvPath() + "/bin/python" + getPythonVersion();
#elif defined(Q_OS_WIN64)
                std::string envPath = getPythonEnvPath();
                std::string interpreterPath = envPath + "/python.exe";

                if (QFile::exists(QString::fromStdString(interpreterPath)))
                    return interpreterPath;
                else
                    return envPath + "/Scripts/python.exe";
#endif
            }
            inline std::string  getPythonLibPath()
            {
#if defined(Q_OS_LINUX)
                return getPythonEnvPath() + "/lib/python" + getPythonVersion();
#elif defined(Q_OS_WIN64)
                return getPythonEnvPath() + "/Lib";
#endif
            }
            inline std::string  getPythonDynLoadPath()
            {
#if defined(Q_OS_LINUX)
                std::string dynloadPath = getPythonEnvPath() + "/lib/python" + getPythonVersion() + "/lib-dynload";
#elif defined(Q_OS_WIN64)
                std::string dynloadPath = getPythonEnvPath() + "/DLLs";
#endif
                QDir qDynloadDir(QString::fromStdString(dynloadPath));
                if (qDynloadDir.exists())
                    return dynloadPath;
                else
                {
                    QSettings ikomiaSettings;
                    const QString PYTHON_ROOT_LIB_PATH("python_dev_root_lib_path");
                    return ikomiaSettings.value(PYTHON_ROOT_LIB_PATH, "").toString().toStdString();
                }
            }
            inline std::string  getPythonSitePackagesPath()
            {
#if defined(Q_OS_LINUX)
                return getPythonEnvPath() + "/lib/python" + getPythonVersion() + "/site-packages";
#elif defined(Q_OS_WIN64)
                return getPythonEnvPath() + "/Lib/site-packages";
#endif
            }
            inline std::string  getPythonBinPath()
            {
#if defined(Q_OS_LINUX)
                return getPythonEnvPath() + "/bin";
#elif defined(Q_OS_WIN64)
                return getPythonEnvPath() + "/Scripts";
#endif
            }
            inline std::string  getPythonIkomiaApiFolder()
            {
                std::string apiDir = Utils::IkomiaApp::getIkomiaFolder() + "/Api";
                QDir qApiDir(QString::fromStdString(apiDir));

                if (qApiDir.exists())
                    return apiDir;
                else
                {
#if defined(Q_OS_LINUX)
                    QString defaultApiPath = QDir::homePath() + "/Developpement/IkomiaApi";
#elif defined(Q_OS_WIN64)
                    QString defaultApiPath = "C:/Developpement/IkomiaApi;";
#endif
                    QSettings ikomiaSettings;
                    const QString IKOMIA_API_PATH("ikomia_python_api_dev_path");
                    return ikomiaSettings.value(IKOMIA_API_PATH, defaultApiPath).toString().toStdString();
                }
            }

            inline void         prepareQCommand(QString& cmd, QStringList& args)
            {
#if defined(Q_OS_LINUX)
                Q_UNUSED(args);
                cmd = QString::fromStdString(getPythonInterpreterPath());
#elif defined(Q_OS_WIN64)
                cmd = "cmd.exe";
                QString pythonExePath = QString::fromStdString(getPythonInterpreterPath());
                args << "/c" << pythonExePath;
#endif
            }

            inline object       runCommand(const std::vector<std::string>& args)
            {
                // DO NOT USE THIS FUNCTION
                // DOES NOT WORK -> problems with keyword arguments...
                CPyEnsureGIL gil;
                object sys = import("sys");
                object subprocess = import("subprocess");

                list pyCommand;
                pyCommand.append(sys.attr("executable"));
                for(size_t i=0; i<args.size(); ++i)
                    pyCommand.append(str(args[i]));

                dict pyArgs;
                pyArgs["stdout"] = subprocess.attr("PIPE");
                pyArgs["stderr"] = subprocess.attr("PIPE");
                pyArgs["text"] = object(true);

                object result = subprocess.attr("run")(pyCommand, **pyArgs);
                return result;
            }
            inline bool         runQCommand(const QStringList& args)
            {
                QString cmd;
                QStringList cmdArgs;
                prepareQCommand(cmd, cmdArgs);

                QProcess proc;
    #if defined(Q_OS_LINUX)
                // Some python packages need to be built with gcc and thus, they need to link with the bundled python library.
                // That's why we pass the right path to our python environment
                QString pythonLibPath = "LDFLAGS=-L" + Utils::IkomiaApp::getQIkomiaFolder() + "/Python/lib";
                QStringList env = QProcess::systemEnvironment();
                env << pythonLibPath;
                proc.setEnvironment(env);
    #endif
                cmdArgs << args;
                proc.start(cmd, cmdArgs);

                if(!proc.waitForFinished(-1))
                {
                    qCritical().noquote() << QString(proc.readAllStandardError());
                    return false;
                }
                else
                {
                    QString output = proc.readAllStandardOutput();
                    QString err = proc.readAllStandardError();

                    if(output.isEmpty() == false)
                    {
                        qInfo().noquote() << output << err;
                        return true;
                    }
                    else
                    {
                        qCritical().noquote() << err;
                        return false;
                    }
                }
            }

            inline void         installRequirements(const QString& txtFile)
            {
                QStringList args = {"-m", "pip", "install", "-r"};
                args << txtFile;
                runQCommand(args);
            }
            inline void         installPackage(const QString& package, const QString& version)
            {
                QStringList args = {"-m", "pip", "install"};
                QString packageWithVersion = package + "==" + version;
                args << packageWithVersion;
                runQCommand(args);
            }
            inline void         uninstallPackage(const QString& package)
            {
                QStringList args = {"-m", "pip", "uninstall", "-y"};
                args << package;
                runQCommand(args);
            }

            inline std::vector<std::pair<std::string, std::string>>    getInstalledModules()
            {
                QString cmd;
                QStringList args;
                std::vector<std::pair<std::string, std::string>> modules;

                prepareQCommand(cmd, args);
                args << "-m" << "pip" << "list" << "--format" << "json";

                QProcess proc;
                proc.start(cmd, args);
                proc.waitForFinished();
                QByteArray out = proc.readAllStandardOutput();
                auto jsonDoc = QJsonDocument::fromJson(out);

                if(jsonDoc.isNull() || jsonDoc.isEmpty() || !jsonDoc.isArray())
                    qWarning() << QObject::tr("Error while gathering Python package list.");

                QJsonArray packages = jsonDoc.array();
                for(int i=0; i<packages.size(); ++i)
                {
                    auto package = packages[i].toObject();
                    auto name = package["name"].toString().toStdString();
                    auto version = package["version"].toString().toStdString();
                    modules.push_back(std::make_pair(name, version));
                }
                return modules;
            }
        }

        namespace User
        {
            inline QString  getUserRoleName(int role)
            {
                QString roleName = QObject::tr("None");
                switch(role)
                {
                    case 0: roleName = QObject::tr("Administrator"); break;
                    case 1: roleName = QObject::tr("User"); break;
                }
                return roleName;
            }
            inline QString getNamespaceDisplayName(const QString& namespacePath)
            {
                QString name;
                QStringList pathItems = namespacePath.split("/", Qt::SkipEmptyParts);

                for (int i=1; i<pathItems.size(); ++i)
                    name += pathItems[i] + "/";

                return name;
            }
        }

        namespace Network
        {
            inline QString getBaseUrl()
            {
                const QString DEFAULT_BASE_URL("default_base_url");
                QSettings ikomiaSettings;
                QString baseURL = ikomiaSettings.value(DEFAULT_BASE_URL).toString();

                if(baseURL.isEmpty() || baseURL == "https://members.ikomia.com")
                {
                    baseURL = "https://scale.ikomia.ai";
                    ikomiaSettings.setValue(DEFAULT_BASE_URL, baseURL);
                }
                return baseURL;
            }
            inline QString getPiwikUrl()
            {
                return "https://ikomia.piwik.pro";
            }
            inline QString getBreakPadServerUrl()
            {
                QSettings ikomiaSettings;
                const QString BREAKPAD_SERVER_URL("breakpad_server_url");
                QString url = ikomiaSettings.value(BREAKPAD_SERVER_URL).toString();

                if(url.isEmpty())
                {
                    url = "http://members.ikomia.com:1127/crashreports";
                    ikomiaSettings.setValue(BREAKPAD_SERVER_URL, url);
                }
                return url;
            }
        }

        namespace Database
        {
            inline QString  getProcessConnectionName()
            {
                return "ProcessConnection";
            }
        }

        namespace Data
        {
            inline QString getExportFormatName(DataFileFormat format)
            {
                switch(format)
                {
                    case DataFileFormat::NONE: return "";
                    case DataFileFormat::TXT: return QObject::tr("Text file(*.txt)");
                    case DataFileFormat::JSON: return QObject::tr("JSON file(*.json)");
                    case DataFileFormat::XML: return QObject::tr("XML file(*.xml)");
                    case DataFileFormat::YAML: return QObject::tr("YAML file(*.yml)");
                    case DataFileFormat::CSV: return QObject::tr("CSV file(*.csv)");
                    case DataFileFormat::BMP: return QObject::tr("Bitmap image file(*.bmp)");
                    case DataFileFormat::JPG: return QObject::tr("JPEG image file(*.jpg)");
                    case DataFileFormat::JP2: return QObject::tr("JP2K image file(*.jp2)");
                    case DataFileFormat::PNG: return QObject::tr("PNG image file(*.png)");
                    case DataFileFormat::TIF: return QObject::tr("TIFF image file(*.tif)");
                    case DataFileFormat::WEBP: return QObject::tr("WebP image file(*.webp)");
                    case DataFileFormat::AVI: return QObject::tr("AVI video file(*.avi)");
                    case DataFileFormat::MPEG: return QObject::tr("MPEG video file(*.mp4)");
                }
                return "";
            }
            inline DisplayType treeItemTypeToDisplayType(TreeItemType type)
            {
                DisplayType displayType = DisplayType::EMPTY_DISPLAY;
                switch(type)
                {
                    case TreeItemType::NONE:
                    case TreeItemType::PROJECT:
                    case TreeItemType::FOLDER:
                    case TreeItemType::DATASET:
                    case TreeItemType::DIMENSION:
                    case TreeItemType::PROCESS:
                    case TreeItemType::RESULT:
                    case TreeItemType::GRAPHICS_LAYER:
                        displayType = DisplayType::EMPTY_DISPLAY;
                        break;

                    case TreeItemType::IMAGE:
                        displayType = DisplayType::IMAGE_DISPLAY;
                        break;

                    case TreeItemType::VIDEO:
                    case TreeItemType::LIVE_STREAM:
                        displayType = DisplayType::VIDEO_DISPLAY;
                        break;
                }
                return displayType;
            }
        }

        namespace Image
        {
            inline QImage colorToMono(const QImage& img)
            {
                QImage imgMono = img;
                for(int i=0; i<imgMono.height(); ++i)
                {
                    QRgb* scanLine = (QRgb*)imgMono.scanLine(i);
                    for(int j=0; j<imgMono.width(); ++j)
                    {
                        QRgb pixel = *scanLine;
                        int val = qGray(pixel);
                        *scanLine = qRgba(val, val, val, qAlpha(pixel));
                        ++scanLine;
                    }
                }
                return imgMono;
            }

            inline QPixmap colorToMono(const QPixmap& pxmap)
            {
                QImage img = pxmap.toImage();
                QImage imgMono = colorToMono(img);
                return QPixmap::fromImage(imgMono);
            }

            inline QPixmap loadPixmap(const QString& path, const QSize& size)
            {
                CImageDataIO io(path.toStdString());
                CMat img = io.read();
                QImage image = CDataConversion::CMatToQImage(img);
                QPixmap pixmap =  QPixmap::fromImage(image);

                if(size.isEmpty() == false)
                    pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                return pixmap;
            }

            inline QPixmap createRoundedPixmap(const QPixmap& pixmap)
            {
                // creating a new transparent pixmap with equal sides
                QPixmap rounded = QPixmap(pixmap.width(), pixmap.height());
                rounded.fill(Qt::transparent);

                // creating circle clip area
                QPainterPath painterPath;
                painterPath.addRoundedRect(rounded.rect(), pixmap.width()*0.1, pixmap.height()*0.1);

                QPainter painter(&rounded);
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setClipPath(painterPath);

                painter.drawPixmap(0, 0, pixmap.width(), pixmap.height(), pixmap);

                return rounded;
            }
        }

        namespace File
        {
            inline QString saveFile(QWidget* pParent, const QString& title, const QString& dir, const QString& filter, const QStringList& extensionList, const QString& defaultExt)
            {
                auto fileName = QFileDialog::getSaveFileName(pParent, title, dir, filter, nullptr, CSettingsManager::dialogOptions());

                if(fileName.isEmpty())
                    return fileName;

                QFileInfo file(fileName);
                auto fileSuffix = file.suffix();

                if(fileSuffix.isEmpty())
                    fileName.append(defaultExt);
                else
                {
                    for(auto&& it : extensionList)
                    {
                        if(fileSuffix == it)
                            return fileName;
                    }

                    fileName.append(defaultExt);
                }

                return fileName;
            }
        }

        namespace Plugin
        {
            inline void installRequirements(const std::string& name)
            {
                std::string pluginDir = Utils::Plugin::getPythonPath() + "/" + name;
                boost::filesystem::path pluginPath(pluginDir);

                if(boost::filesystem::exists(pluginPath) == false)
                {
                    std::string error = "Algorithm " + name + " not found in " + pluginDir;
                    throw CException(CoreExCode::NOT_FOUND, error);
                }

                //Requirements files
                std::set<QString> requirements;
                QString qpluginDir = QString::fromStdString(pluginDir);
                QDir dir(qpluginDir);
                QRegularExpression re("[rR]equirements[0-9]*.txt");

                foreach (QString fileName, dir.entryList(QDir::Files|QDir::NoSymLinks))
                {
                    if(fileName.contains(re))
                        requirements.insert(fileName);
                }

                for(auto&& name : requirements)
                {
                    QString requirementFile = qpluginDir + "/" + name;
                    Utils::print("Algorithm dependencies installation from " + requirementFile, QtInfoMsg);
                    Utils::Python::installRequirements(requirementFile);
                }
            }
        }
    }
}

#endif // APPTOOLS_HPP
