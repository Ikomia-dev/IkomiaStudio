#ifndef CMAINTITLEBAR_H
#define CMAINTITLEBAR_H

/**
 * @file      CMainTitleBar.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CMainTitleBar definition
 *
 * @details   Details
 */

#include <QFrame>

class QHBoxLayout;
class QPushButton;

/**
 * @brief
 *
 */
class CMainTitleBar : public QFrame
{
    Q_OBJECT

    public:

        explicit CMainTitleBar(int size, QWidget* parent = Q_NULLPTR);

        void    setUser(const QString& name);

    signals:

        void    close();

        void    minimize();

        void    expand();

    private:

        void    initTitle();
        void    initButtons();
        void    initConnections();

    private:

        QLabel*         m_pLabelTitle = nullptr;
        QHBoxLayout*    m_pLayout = nullptr;
        QPushButton*    m_pClose = nullptr;
        QPushButton*    m_pMinimize = nullptr;
        QPushButton*    m_pExpand = nullptr;
        int             m_size = 20;
};

#endif // CMAINTITLEBAR_H
