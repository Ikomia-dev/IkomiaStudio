"""
Main Ikomia plugin module.
Ikomia Studio and Ikomia API use it to load algorithms dynamically.
"""
from ikomia import dataprocess
from _PluginName_._PluginName__process import _PluginClassName_Factory
from _PluginName_._PluginName__process import _PluginClassName_ParamFactory


class IkomiaPlugin(dataprocess.CPluginProcessInterface):
    """
    Interface class to integrate the process with Ikomia application.
    Inherits PyDataProcess.CPluginProcessInterface from Ikomia API.
    """
    def __init__(self):
        dataprocess.CPluginProcessInterface.__init__(self)

    def get_process_factory(self):
        """Instantiate process object."""
        return _PluginClassName_Factory()

    def get_widget_factory(self):
        """Instantiate associated widget object."""
        from _PluginName_._PluginName__widget import _PluginClassName_WidgetFactory
        return _PluginClassName_WidgetFactory()

    def get_param_factory(self):
        """Instantiate algorithm parameters object."""
        return _PluginClassName_ParamFactory()
