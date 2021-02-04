#ifndef _PluginNameUpperCase__H
#define _PluginNameUpperCase__H

#include <QObject>
#include <memory>
#include "_PluginName_Global.hpp"
#include "CPluginProcessInterface.hpp"
#include "Protocol/CProtocolTaskParam.hpp"

#include "Core/CWidgetFactory.hpp"
#include "_ProcessBaseClassHeader_"
#include "_WidgetBaseClassHeader_"

//------------------------------
//----- C_PluginName_Param
//------------------------------
class C_PluginName_Param: public CProtocolTaskParam
{
    public:

        C_PluginName_Param();

        //Set parameters values from Ikomia application
        void        setParamMap(const UMapString& paramMap) override;

        //Send parameters values to Ikomia application
        UMapString  getParamMap() const override;
};

//------------------------------
//----- C_PluginName_
//------------------------------
class _PluginNameUpperCase_SHARED_EXPORT C_PluginName_ : public _ProcessBaseClass_
{
    public:

        C_PluginName_();
        C_PluginName_(const std::string& name, const std::shared_ptr<C_PluginName_Param>& pParam);

        //Function returning the number of progress steps frot this process
        //This is handled by the main progress bar of Ikomia application
        size_t          getProgressSteps() override;

        void            run() override;
};

class _PluginNameUpperCase_SHARED_EXPORT C_PluginName_Factory : public CProcessFactory
{
    public:

        C_PluginName_Factory()
        {
            //Mandatory fields
            m_info.m_name = "_PluginName_";
            m_info.m_description = "Your description";
            m_info.m_authors = "Authors";
            m_info.m_version = "1.0.0";
            //Relative path -> as displayed in Ikomia application process tree
            m_info.m_path = "Plugins";
            m_info.m_iconPath = "";
            //For search
            m_info.m_keywords = "Your keywords here";
            m_info.m_language = CProcessInfo::CPP;
        }

        virtual ProtocolTaskPtr create(const ProtocolTaskParamPtr& pParam) override
        {
            auto pDerivedParam = std::dynamic_pointer_cast<C_PluginName_Param>(pParam);
            if(pDerivedParam != nullptr)
                return std::make_shared<C_PluginName_>(m_info.m_name, pDerivedParam);
            else
                return create();
        }
        virtual ProtocolTaskPtr create() override
        {
            auto pDerivedParam = std::make_shared<C_PluginName_Param>();
            assert(pDerivedParam != nullptr);
            return std::make_shared<C_PluginName_>(m_info.m_name, pDerivedParam);
        }
};

//------------------------------
//----- C_PluginName_Widget
//------------------------------
class _PluginNameUpperCase_SHARED_EXPORT C_PluginName_Widget : public _WidgetBaseClass_
{
    public:

        C_PluginName_Widget(QWidget *parent = Q_NULLPTR);
        C_PluginName_Widget(ProtocolTaskParamPtr pParam, QWidget *parent = Q_NULLPTR);

    protected:

        virtual void init();

    private:

        std::shared_ptr<C_PluginName_Param> m_pParam;
};

class _PluginNameUpperCase_SHARED_EXPORT C_PluginName_WidgetFactory : public CWidgetFactory
{
    public:

        C_PluginName_WidgetFactory()
        {
            m_name = "_PluginName_";
        }

        virtual ProtocolTaskWidgetPtr   create(ProtocolTaskParamPtr pParam)
        {
            return std::make_shared<C_PluginName_Widget>(pParam);
        }
};

//------------------------------
//----- Global plugin interface
//------------------------------
class _PluginNameUpperCase_SHARED_EXPORT C_PluginName_Interface : public QObject, public CPluginProcessInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "ikomia.plugin.process")
    Q_INTERFACES(CPluginProcessInterface)

    public:

        virtual std::shared_ptr<CProcessFactory> getProcessFactory()
        {
            return std::make_shared<C_PluginName_Factory>();
        }

        virtual std::shared_ptr<CWidgetFactory> getWidgetFactory()
        {
            return std::make_shared<C_PluginName_WidgetFactory>();
        }
};

#endif // _PluginNameUpperCase__H
