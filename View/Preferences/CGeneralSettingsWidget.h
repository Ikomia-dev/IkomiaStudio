#ifndef CGENERALSETTINGSWIDGET_H
#define CGENERALSETTINGSWIDGET_H

#include <QWidget>

class QCheckBox;

class CGeneralSettingsWidget : public QWidget
{
       Q_OBJECT

    public:

        explicit    CGeneralSettingsWidget(QWidget* parent = nullptr);

    signals:

        void        doEnableTutorialHelper(bool bEnable);
        void        doEnableNativeDialog(bool bEnable);

    public slots:

        void        onEnableTutorialHelper(bool bEnable);
        void        onEnableNativeDialog(bool bEnable);

    private:

        void        initLayout();
        void        initConnections();

    private:

        // Tutorials standby
        QCheckBox*  m_pCheckTuto = nullptr;
        QCheckBox*  m_pCheckNative = nullptr;
};

#endif // CGENERALSETTINGSWIDGET_H
