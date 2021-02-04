#ifndef CMODULEDOCKWIDGET_H
#define CMODULEDOCKWIDGET_H

#include <QDockWidget>
#include "Main/forwards.hpp"

class CModuleDockWidget : public QDockWidget
{
    public:

        CModuleDockWidget(const QString &title, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
        CModuleDockWidget(QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());

        void                addModuleWidget(QWidget* pWidget, const QIcon &icon);

        void                removeModuleWidget(QWidget* pWidget);

        QToolButton*        getModuleBtn(int id);

        bool                isModuleOpen(int id);

        void                updateWindowTitle();

        void                showModule(int index);

    private slots:

        void                onActivateScrollBar();

    private:

        void                init();

        QPropertyAnimation* createAnimation(QByteArray name, QVariant start, QVariant end, int duration);

        void                animate();

        void                toggleModule(int index);

        void                desactivateScrollBar();

    private:

        bool                            m_bIsOpened = false;
        int                             m_currentHeight = 0;
        int                             m_minHeight = 0;
        QWidget*                        m_pView = nullptr;
        CToolbarBorderLayout*           m_pLayout = nullptr;
        QStackedWidget*                 m_pModuleViews = nullptr;
        QVector<QPropertyAnimation*>    m_animations;
        QList<QToolButton*>             m_moduleBtnList;
};

#endif // CMODULEDOCKWIDGET_H
