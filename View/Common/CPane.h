#ifndef CPANE_H
#define CPANE_H

/**
 * @file      CPane.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CPane definition
 *
 * @details   Details
 */

#include <QWidget>
#include <QVBoxLayout>
#include "Main/forwards.hpp"

/**
 * @brief
 *
 */
class CPane : public QWidget
{
    public:
        /**
         * @brief
         *
         * @param parent
         */
        CPane(QWidget* parent = Q_NULLPTR);

        void        setAnimation(QByteArray animation, int minValue, int maxValue);

        void        animate();

    protected:

        QVBoxLayout*        m_pLayout = nullptr;
        bool                m_bIsOpened = false;
        QByteArray          m_animationName = "";
        int                 m_animationMinValue = 0;
        int                 m_animationMaxValue = 0;
};

#endif // CPANE_H
