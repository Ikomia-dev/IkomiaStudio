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
