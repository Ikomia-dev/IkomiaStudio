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

#include "CVTKSelectVisibleLayersDlg.h"

#include <vector>
#include <QDialog>
#include <QDialogButtonBox>
#include <QObject>
#include <QWidget>



CVTKSelectVisibleLayersDlg::CVTKSelectVisibleLayersDlg(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    setWindowTitle("Layers to display...");

    // Initialization of the layout's widgets
    initLayout();

    // Initialization of the layout's connections
    initConnections();
}

void CVTKSelectVisibleLayersDlg::initLayout()
{
    // Label used to display informations
    m_lblInfo = new QLabel("No layer to display");

    // List of layers
    m_lstLayers = new QListWidget;

    // Buttons Ok / Cancel
    m_btbButtonsOkCancel = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Main layout (vertical)
    m_lytMain = new QVBoxLayout;
    m_lytMain->addWidget(m_lblInfo);
    m_lytMain->addWidget(m_lstLayers);
    m_lytMain->addWidget(m_btbButtonsOkCancel);

    setLayout(m_lytMain);
}

void CVTKSelectVisibleLayersDlg::initConnections()
{
    QObject::connect(m_btbButtonsOkCancel, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(m_btbButtonsOkCancel, &QDialogButtonBox::rejected, this, &QDialog::reject);
    QObject::connect(m_lstLayers,          &QListWidget::itemActivated, this, &CVTKSelectVisibleLayersDlg::onLstLayersItemActivated);
}

void CVTKSelectVisibleLayersDlg::setLstLayers(const std::vector<CScene3dLayer>& lstLayers)
{
    if(lstLayers.size() == 0)
    {
        // No layer to display
        m_lblInfo->setText("No layer to display");
        m_lstLayers->setVisible(false);
    }
    else
    {
        // There are some layers to display
        m_lblInfo->setText("Select visible layers");
        m_lstLayers->setVisible(true);

        // The old list is clear
        m_lstLayers->clear();

        // For each layer...
        for(int i = 0; i < lstLayers.size(); ++i)
        {
            // ... a new item is created. Each item is made up of
            // a checkbox and a label (= index of the current layer)
            QListWidgetItem *itemLayer = new QListWidgetItem;
            itemLayer->setText(QString::number(i));
            itemLayer->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
            itemLayer->setCheckState(lstLayers[i].isVisible() ? Qt::Checked : Qt::Unchecked);
            m_lstLayers->addItem(itemLayer);
        }
    }
}

std::vector<bool> CVTKSelectVisibleLayersDlg::getCheckedLayers()
{
    std::vector<bool> lstCheckedLayers;

    // For each item...
    for(int i = 0; i < m_lstLayers->count(); ++i)
    {
        // the value 'true' is put inside the result vector if the
        // checkbox is checked, 'false' otherwise
        QListWidgetItem *itemLayer = m_lstLayers->item(i);
        lstCheckedLayers.push_back(itemLayer->checkState() == Qt::Checked);
    }

    return lstCheckedLayers;
}

void CVTKSelectVisibleLayersDlg::onLstLayersItemActivated(QListWidgetItem *item)
{
    // We toggle the current checkbox associated to this item
    item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
}
