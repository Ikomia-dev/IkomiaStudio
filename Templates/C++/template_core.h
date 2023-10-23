/*
 * Copyright (C) 2021 Ikomia SAS
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

#ifndef _PluginNameUpperCase__H
#define _PluginNameUpperCase__H

#include <QObject>
#include <memory>
#include "_PluginName_Global.hpp"
#include "CPluginProcessInterface.hpp"
#include "Workflow/CWorkflowTaskParam.hpp"

#include "Core/CWidgetFactory.hpp"
#include "_ProcessBaseClassHeader_"
#include "_WidgetBaseClassHeader_"

//------------------------------
//----- C_PluginName_Param
//------------------------------
class C_PluginName_Param: public CWorkflowTaskParam
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

class _PluginNameUpperCase_SHARED_EXPORT C_PluginName_Factory : public CTaskFactory
{
    public:

        C_PluginName_Factory()
        {
            //Mandatory fields
            m_info.m_name = "_PluginName_";
            m_info.m_shortDescription = "Your description";
            m_info.m_authors = "Authors";
            m_info.m_article = "";
            m_info.m_articleUrl = "";
            m_info.m_journal = "";
            m_info.m_year = 2020;
            m_info.m_version = "1.0.0";
            //Relative path -> as displayed in Ikomia application process tree
            m_info.m_path = "Plugins";
            m_info.m_iconPath = "";
            //For search
            m_info.m_keywords = "Your keywords here";
            m_info.m_language = ApiLanguage::CPP;
        }

        virtual WorkflowTaskPtr create(const WorkflowTaskParamPtr& pParam) override
        {
            auto pDerivedParam = std::dynamic_pointer_cast<C_PluginName_Param>(pParam);
            if(pDerivedParam != nullptr)
                return std::make_shared<C_PluginName_>(m_info.m_name, pDerivedParam);
            else
                return create();
        }
        virtual WorkflowTaskPtr create() override
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
        C_PluginName_Widget(WorkflowTaskParamPtr pParam, QWidget *parent = Q_NULLPTR);

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

        virtual WorkflowTaskWidgetPtr   create(WorkflowTaskParamPtr pParam)
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

        virtual std::shared_ptr<CTaskFactory> getProcessFactory()
        {
            return std::make_shared<C_PluginName_Factory>();
        }

        virtual std::shared_ptr<CWidgetFactory> getWidgetFactory()
        {
            return std::make_shared<C_PluginName_WidgetFactory>();
        }
};

#endif // _PluginNameUpperCase__H
