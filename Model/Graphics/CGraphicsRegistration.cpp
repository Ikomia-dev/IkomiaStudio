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

#include "CGraphicsRegistration.h"
#include "Graphics/CGraphicsPoint.h"
#include "Graphics/CGraphicsEllipse.h"
#include "Graphics/CGraphicsRectangle.h"
#include "Graphics/CGraphicsPolygon.h"
#include "Graphics/CGraphicsPolyline.h"
#include "Graphics/CGraphicsComplexPolygon.h"
#include "Graphics/CGraphicsText.h"

CGraphicsRegistration::CGraphicsRegistration()
{
    registerItem(std::make_shared<CGraphicsPointFactory>());
    registerItem(std::make_shared<CGraphicsEllipseFactory>());
    registerItem(std::make_shared<CGraphicsRectangleFactory>());
    registerItem(std::make_shared<CGraphicsPolygonFactory>());
    registerItem(std::make_shared<CGraphicsPolylineFactory>());
    registerItem(std::make_shared<CGraphicsComplexPolygonFactory>());
    registerItem(std::make_shared<CGraphicsTextFactory>());
}

const CGraphicsAbstractFactory &CGraphicsRegistration::getFactory() const
{
    return m_factory;
}

void CGraphicsRegistration::registerItem(const std::shared_ptr<CGraphicsFactory> &pFactory)
{
    assert(pFactory);

    m_factory.getList().push_back(pFactory);
    //Passage par lambda -> pFactory par valeur pour assurer la portée du pointeur
    auto pCreatorFunc = [pFactory](QGraphicsItem* pParent){ return pFactory->create(pParent); };
    m_factory.registerCreator(pFactory->getType(), pCreatorFunc);
}
