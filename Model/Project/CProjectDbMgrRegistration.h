#ifndef CPROJECTDBMGRREGISTRATION_H
#define CPROJECTDBMGRREGISTRATION_H

#include "CProjectDbMgrInterface.hpp"

class CProjectDbMgrRegistration
{
    public:

        CProjectDbMgrRegistration();

        const CProjectDbMgrAbstractFactory& getFactory() const;

        void                                registerManager(const std::shared_ptr<CProjectDbMgrFactory>& factoryPtr);

    private:

        CProjectDbMgrAbstractFactory    m_factory;
};

#endif // CPROJECTDBMGRREGISTRATION_H
