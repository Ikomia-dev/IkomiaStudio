#ifndef CTRAININGMONITORING_H
#define CTRAININGMONITORING_H

#include <QObject>
#include <QCoreApplication>
#include <QThread>
#include <QtNetwork/QNetworkAccessManager>
#include "UtilsGlobal.hpp"


class UTILSSHARED_EXPORT CTrainingMonitoring: public QObject
{
    Q_OBJECT

    using MapString = std::map<std::string, std::string>;

    public:

        CTrainingMonitoring(QNetworkAccessManager* pNetworkMgr);

        void    setMLflowConfig(const std::string& key, const std::string& value);
        void    setTensorboadConfig(const std::string& key, const std::string& value);

        void    checkMLflowServer();
        void    checkTensorboardServer();

    private:

        MapString               m_mlflowConfig;
        MapString               m_tensorboardConfig;
        QNetworkAccessManager*  m_pNetworkMgr = nullptr;
};

#endif // CTRAININGMONITORING_H
