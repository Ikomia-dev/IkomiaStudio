#ifndef CPLUGINTOOLS_H
#define CPLUGINTOOLS_H

#include "CPluginProcessInterface.hpp"

namespace Ikomia
{
    namespace Utils
    {
        class CPluginTools
        {
            public:

                CPluginTools();

                static std::string              getTransferPath();
                static std::string              getDirectory(const std::string& name, int language);
                static QString                  getCppPluginFolder(const QString& name);
                static QString                  getPythonPluginFolder(const QString& name);

                static boost::python::object    loadPythonModule(const std::string& name);

            private:

                static QString                  getPythonLoadedPluginFolder(const QString &name);
        };
    }
}

#endif // CPLUGINTOOLS_H
