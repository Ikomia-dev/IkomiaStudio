#ifndef CNOTIFICATIONPOPUP_H
#define CNOTIFICATIONPOPUP_H

#include <QDialog>

class CLabelBtn;
class QLabel;
class QToolButton;
class QGridLayout;

class CNotificationPopup : public QDialog
{
    Q_OBJECT

    public:

        CNotificationPopup(QWidget* pParent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ~CNotificationPopup();

        void    showPopup(const QString& title, const QString description, const QIcon& icon = QIcon());

        void    addWidget(QWidget* pWidget, int x, int y);
        void    removeWidget(QWidget* pWidget);

        void    setDescriptionPos(int x, int y);
        void    setPadding(size_t padding);
        void    setDescription(const QString& text);

        void    mousePressEvent(QMouseEvent*);

    signals:

        void    doClose();
        void    doClicked();

    public slots:

        void    onClosePopup();

    private:

        void    initLayout();
        void    initConnections();

        void    updatePos();

    private:

        QToolButton*    m_pCloseBtn  = nullptr;
        QGridLayout*    m_pBodyLayout = nullptr;
        CLabelBtn*      m_pTitle  = nullptr;
        QLabel*         m_pDescription = nullptr;
        size_t          m_padding = 0;
        bool            m_bDescription = false;
};

class CNotificationPopupFactory
{
    public:
        inline CNotificationPopup* create(QWidget* pParent = nullptr)
        {
            return new CNotificationPopup(pParent);
        }
};

#endif // CNOTIFICATIONPOPUP_H
