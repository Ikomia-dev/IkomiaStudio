/*
 * Copyright (C) 2023 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CVTKSELECTVISIBLELAYERSDLG_H
#define CVTKSELECTVISIBLELAYERSDLG_H


#include <vector>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QObject>
#include <QVBoxLayout>
#include <QWidget>

#include "IO/CScene3dIO.h"



/**
 * @brief Dialog box used to select visible layers of a 3D scene.
 */
class CVTKSelectVisibleLayersDlg : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     */
    CVTKSelectVisibleLayersDlg(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /**
     * @brief Initialize the layout's widgets.
     */
    void initLayout();

    /**
     * @brief Initialize the layout's connections.
     */
    void initConnections();

    /**
     * @brief Set the layer list, coming from a 3D scene.
     * @param lstLayers: list of layers
     */
    void setLstLayers(const std::vector<CScene3dLayer>& lstLayers);

    /**
     * @brief Return a list of booleans. A 'true' value indicates that
     * associated layer (= same index) must be displayed.
     */
    std::vector<bool> getCheckedLayers();

public slots:
    void onLstLayersItemActivated(QListWidgetItem *item);

private:
    /**
     * @brief Label used to display some informations.
     */
    QLabel *m_lblInfo;

    /**
     * @brief List of layers, made up of checkboxes and labels.
     */
    QListWidget* m_lstLayers;

    /**
     * @brief Buttons 'Ok' and 'Cancel'.
     */
    QDialogButtonBox *m_btbButtonsOkCancel;

    /**
     * @brief Main layout.
     */
    QVBoxLayout *m_lytMain;
};

#endif // CVTKSELECTVISIBLELAYERSDLG_H
