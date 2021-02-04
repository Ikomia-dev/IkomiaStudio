#ifndef CPROTOCOLSETTINGSWIDGET_H
#define CPROTOCOLSETTINGSWIDGET_H

#include <QWidget>

class CBrowseFileWidget;

class CProtocolSettingsWidget : public QWidget
{
    Q_OBJECT

    public:

        CProtocolSettingsWidget(QWidget* parent = nullptr);

    signals:

        void    doSetSaveFolder(const QString& path);

    public slots:

        void    onSetSaveFolder(const QString& path);

    private:

        void    initLayout();
        void    initConnections();

    private:

        CBrowseFileWidget* m_pBrowseWidget = nullptr;
};

#endif // CPROTOCOLSETTINGSWIDGET_H
