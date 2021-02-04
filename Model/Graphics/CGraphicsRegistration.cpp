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
