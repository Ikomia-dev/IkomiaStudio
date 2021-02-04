#ifndef CGRAPHICSREGISTRATION_H
#define CGRAPHICSREGISTRATION_H

#include "Graphics/CGraphicsItem.hpp"

class CGraphicsRegistration
{
    public:

        CGraphicsRegistration();

        const CGraphicsAbstractFactory& getFactory() const;

        void                            registerItem(const std::shared_ptr<CGraphicsFactory>& pFactory);

    private:

        CGraphicsAbstractFactory    m_factory;
};

#endif // CGRAPHICSREGISTRATION_H
