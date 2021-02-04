#ifndef CDATAVIEWLAYOUT_H
#define CDATAVIEWLAYOUT_H

/**
 * @file      CDataDisplayLayout.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CDataDisplayLayout definition
 *
 * @details   Details
 */

#include <QGridLayout>

/**
 * @brief
 *
 */
class CDataDisplayLayout : public QGridLayout
{
    public:
        /**
         * @brief
         *
         * @param parent
         */
        CDataDisplayLayout(QWidget* parent = nullptr);
};

#endif // CDATAVIEWLAYOUT_H
