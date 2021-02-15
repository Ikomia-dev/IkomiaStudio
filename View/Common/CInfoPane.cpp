// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CInfoPane.h"
#include <QVBoxLayout>
#include <QToolBox>
#include <Widgets/QtPropertyBrowser/qtgroupboxpropertybrowser.h>

CInfoPane::CInfoPane()
{
    initLayout();
}

void CInfoPane::onDisplayImageInfo(const VectorPairString &infoList)
{
    fillProperties(infoList, tr("Image properties"));
}

void CInfoPane::onDisplayVideoInfo(const VectorPairString &infoList)
{
    fillProperties(infoList, tr("Video properties"));
}

void CInfoPane::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    emit doEnableInfo(true);
}

void CInfoPane::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    emit doEnableInfo(false);
}

void CInfoPane::initLayout()
{
    m_pPropertyBrowser = new QtTreePropertyBrowser;
    m_pVariantManager = new QtVariantPropertyManager(this);
    //QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory();
    //m_pPropertyBrowser->setFactoryForManager(m_pVariantManager, variantFactory);
    m_pPropertyBrowser->setPropertiesWithoutValueMarked(true);
    //m_pPropertyBrowser->setRootIsDecorated(true);

    m_pInfoToolBox = new QToolBox;

    m_pInfoToolBox->addItem(m_pPropertyBrowser, tr("Image properties"));

    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(m_pInfoToolBox);

    setLayout(pLayout);
}

void CInfoPane::fillProperties(const VectorPairString &infoList, const QString& title)
{
    m_infoList = infoList;
    // Clear image properties for new values
    m_pPropertyBrowser->clear();
    // Top item for common properties
    QtProperty* pTopItem = m_pVariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), title);
    // Add top item to browser
    m_pPropertyBrowser->addProperty(pTopItem);

    for(auto it=m_infoList.begin(); it!=m_infoList.end(); ++it)
    {
        if(it->first == "Metadata")
        {
            QtProperty* pMetadataItem = m_pVariantManager->addProperty(QtVariantPropertyManager::groupTypeId(), QString::fromStdString(it->second));
            // Add top item to browser
            m_pPropertyBrowser->addProperty(pMetadataItem);
            pTopItem = pMetadataItem;
        }
        else
        {
            QtVariantProperty *item = m_pVariantManager->addProperty(QVariant::String, QString::fromStdString(it->first));
            item->setValue(QString::fromStdString(it->second));
            pTopItem->addSubProperty(item);
        }
    }

    // Set browser line color
    auto list = m_pPropertyBrowser->topLevelItems();
    QPalette p = qApp->palette();
    for(auto&& it : list)
        m_pPropertyBrowser->setBackgroundColor(it, p.base().color());
}

void CInfoPane::fillPropertySize(QtProperty* pItem)
{
    auto widthIt = std::find_if(m_infoList.begin(), m_infoList.end(), [&](const PairString& elt){ return elt.first == tr("Width").toStdString(); });
    auto heightIt = std::find_if(m_infoList.begin(), m_infoList.end(), [&](const PairString& elt){ return elt.first == tr("Height").toStdString(); });

    if(widthIt != m_infoList.end() && heightIt != m_infoList.end())
    {
        QtVariantProperty *item = m_pVariantManager->addProperty(QVariant::Size, tr("Size"));
        item->setValue(QSize(std::stoi(widthIt->second), std::stoi(heightIt->second)));
        pItem->addSubProperty(item);
    }
}

