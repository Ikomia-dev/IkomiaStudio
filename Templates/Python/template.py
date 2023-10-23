from ikomia import dataprocess


# --------------------
# - Interface class to integrate the process with Ikomia application
# - Inherits PyDataProcess.CPluginProcessInterface from Ikomia API
# --------------------
class IkomiaPlugin(dataprocess.CPluginProcessInterface):

    def __init__(self):
        dataprocess.CPluginProcessInterface.__init__(self)

    def get_process_factory(self):
        # Instantiate algorithm object
        from _PluginName_._PluginName__process import _PluginClassName_Factory
        return _PluginClassName_Factory()

    def get_widget_factory(self):
        # Instantiate associated widget object
        from _PluginName_._PluginName__widget import _PluginClassName_WidgetFactory
        return _PluginClassName_WidgetFactory()
