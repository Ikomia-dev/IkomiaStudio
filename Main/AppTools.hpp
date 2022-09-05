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
#include "Core/CTaskInfo.h"
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

            const std::string _python_bin_prod_version = "3.7m";
            const std::string _python_lib_prod_version = "3.7";

            inline std::string  getDevBinVersion()
            {
                QSettings ikomiaSettings;
                const QString PYTHON_BIN_VERSION("python_bin_version");
                QString binVersion = ikomiaSettings.value(PYTHON_BIN_VERSION).toString();

                if(binVersion.isEmpty())
                {
                    binVersion = "3.9";
                    ikomiaSettings.setValue(PYTHON_BIN_VERSION, binVersion);
                }
                return binVersion.toStdString();
            }
            inline std::string  getDevLibVersion()
            {
                QSettings ikomiaSettings;
                const QString PYTHON_LIB_VERSION("python_lib_version");
                QString libVersion = ikomiaSettings.value(PYTHON_LIB_VERSION).toString();

                if(libVersion.isEmpty())
                {
                    libVersion = "3.9";
                    ikomiaSettings.setValue(PYTHON_LIB_VERSION, libVersion);
                }
                return libVersion.toStdString();
            }
            inline std::set<std::string> getImportedModules()
            {
                CPyEnsureGIL gil;
                object main_module = import("__main__");
                object main_namespace = main_module.attr("__dict__");

                str code
                (
                    "import types\n\n"
                    "def imported_modules():\n"
                    "   modules = list()\n"
                    "   for name, val in globals().items():\n"
                    "       if isinstance(val, types.ModuleType):\n"
                    "           modules.append(val.__name__)\n"
                    "   return modules"
                );
                exec(code, main_namespace, main_namespace);
                object imported_modules = main_namespace["imported_modules"];
                object modules = imported_modules();

                std::set<std::string> importedModules;
                for(int i=0; i<len(modules); ++i)
                    importedModules.insert(extract<std::string>(modules[i]));

                return importedModules;
            }

            inline void         prepareQCommand(QString& cmd, QStringList& args)
            {
                QString userPythonFolder = Utils::IkomiaApp::getQIkomiaFolder() + "/Python";
                QDir pythonDir(userPythonFolder);

                if(pythonDir.exists())
                {
    #if defined(Q_OS_WIN64)
                    cmd = "cmd.exe";
                    QString pythonExePath = userPythonFolder + "/python.exe";
                    args << "/c" << pythonExePath;
    #elif defined(Q_OS_LINUX)
                    Q_UNUSED(args);
                    cmd = userPythonFolder + "/bin/python" + QString::fromStdString(_python_bin_prod_version);
    #elif defined(Q_OS_MACOS)
                    Q_UNUSED(args);
                    cmd = userPythonFolder + "/bin/python" + QString::fromStdString(_python_bin_prod_version);
    #endif
                }
                else
                {
    #if defined(Q_OS_WIN64)
                    cmd = "cmd.exe";
                    args << "/c" << "python";
    #elif defined(Q_OS_LINUX)
                    Q_UNUSED(args);
                    cmd = "python" + QString::fromStdString(getDevBinVersion());
    #elif defined(Q_OS_MACOS)
                    Q_UNUSED(args);
                    cmd = "python" + QString::fromStdString(getDevBinVersion());;
    #endif
                }
            }

            inline void         runScript(const std::string& script)
            {
                CPyEnsureGIL gil;
                str pyScript(script);
                object main_module = import("__main__");
                object main_namespace = main_module.attr("__dict__");
                exec(pyScript, main_namespace, main_namespace);
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

            inline bool         isModuleImported(const std::string& name)
            {
                CPyEnsureGIL gil;
                object main_module = import("__main__");
                object main_namespace = main_module.attr("__dict__");

                str code
                (
                    "import types\n\n"
                    "def imported_modules():\n"
                    "   modules = list()\n"
                    "   for name, val in globals().items():\n"
                    "       if isinstance(val, types.ModuleType):\n"
                    "           modules.append(val.__name__)\n"
                    "   return modules"
                );
                exec(code, main_namespace, main_namespace);
                object imported_modules = main_namespace["imported_modules"];
                object modules = imported_modules();

                for(int i=0; i<len(modules); ++i)
                {
                    std::string moduleName = extract<std::string>(modules[i]);
                    if(moduleName == name)
                        return true;
                }
                return false;
            }
            inline bool         isFolderModule(const std::string& folder)
            {
                boost::filesystem::directory_iterator iter(folder), end;
                for(; iter != end; ++iter)
                {
                    if(iter->path().filename() == "__init__.py")
                        return true;
                }
                return false;
            }

            inline object       reloadModule(const std::string& name)
            {
                CPyEnsureGIL gil;
                object main_module = boost::python::import("__main__");
                object main_namespace = main_module.attr("__dict__");
                QString script = QString("import importlib\nimportlib.reload(%1)").arg(QString::fromStdString(name));
                str pyScript(script.toStdString());
                exec(pyScript, main_namespace, main_namespace);
                return main_namespace[name];
            }

            inline void         unloadModule(const std::string& name, bool bStrictCompare)
            {
                CPyEnsureGIL gil;
                object main_module = boost::python::import("__main__");
                object main_namespace = main_module.attr("__dict__");

                QString script;

                if(bStrictCompare)
                {
                    script = QString(
                            "import sys\n"
                            "modules_to_delete = [m for m in sys.modules.keys() if '%1' == m]\n"
                            "for m in modules_to_delete: del(sys.modules[m])\n"
                            ).arg(QString::fromStdString(name));
                }
                else
                {
                    script = QString(
                            "import sys\n"
                            "modules_to_delete = [m for m in sys.modules.keys() if '%1' in m]\n"
                            "for m in modules_to_delete: del(sys.modules[m])\n"
                            ).arg(QString::fromStdString(name));

                }
                str pyScript(script.toStdString());
                exec(pyScript, main_namespace, main_namespace);
            }

            inline void         installRequirements(const QString& txtFile)
            {
                QStringList args = {"-m", "pip", "install", "-r"};
                args << txtFile;
                runQCommand(args);
            }

            inline void         uninstallPackage(const QString& package)
            {
                QStringList args = {"-m", "pip", "uninstall", "-y"};
                args << package;
                runQCommand(args);
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
        }

        namespace Network
        {
            inline QString getBaseUrl()
            {
                const QString DEFAULT_BASE_URL("default_base_url");
                QSettings ikomiaSettings;
                QString baseURL = ikomiaSettings.value(DEFAULT_BASE_URL).toString();

                if(baseURL.isEmpty())
                {
                    baseURL = "https://ikomia.com";
                    ikomiaSettings.setValue(DEFAULT_BASE_URL, baseURL);
                }
                return baseURL;
            }
            inline QString getMatomoUrl()
            {
                return "https://mamoto.ikomia.com";
            }
            inline QString getBreakPadServerUrl()
            {
                QSettings ikomiaSettings;
                const QString BREAKPAD_SERVER_URL("breakpad_server_url");
                QString url = ikomiaSettings.value(BREAKPAD_SERVER_URL).toString();

                if(url.isEmpty())
                {
                    url = "http://ikomia.com:1127/crashreports";
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
    }
}

#endif // APPTOOLS_HPP
