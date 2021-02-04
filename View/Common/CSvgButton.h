#ifndef CSVGBUTTON_H
#define CSVGBUTTON_H

#include <QToolButton>

class QSvgWidget;

class CSvgButton : public QToolButton
{
    public:
        CSvgButton(const QString& filePath, bool isCircular = false, QWidget* parent = nullptr);
        ~CSvgButton();

        void setImage(const QString& filePath);
        void setCircular(bool bIsCircular);

    private:
        void initLayout();

    private:
        QSvgWidget* m_pSvgWidget = nullptr;
        QString m_filePath = "";
        bool m_bIsCircular = false;
};

#endif // CSVGBUTTON_H
