#ifndef _PluginNameUpperCase__GLOBAL_H
#define _PluginNameUpperCase__GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(_PluginNameUpperCase__LIBRARY)
#  define _PluginNameUpperCase_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define _PluginNameUpperCase_SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // _PluginNameUpperCase__GLOBAL_H