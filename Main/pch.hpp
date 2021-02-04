#ifndef PCH_HPP
#define PCH_HPP

#ifndef HAVE_SNPRINTF
    #define HAVE_SNPRINTF
#endif

#include <QtWidgets>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector3D>
#include <QTime>
#include <QPropertyAnimation>
#include <QFileSystemModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QColor>
#include <vector>
#include <set>
#include <unordered_map>
#include <map>
#include <memory>
#include <functional>
#include <QDebug>
#include <cassert>
#include <CException.h>
#include "Main/CoreDefine.hpp"
#include "Main/AppDefine.hpp"

//Avoid conflict with Qt slots keyword
#undef slots
#include <boost/python.hpp>
#define slots

#endif // PCH_HPP
