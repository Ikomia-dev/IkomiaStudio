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

#include "C3dAnimation.h"
#include <QFile>

//------------------------------//
//- Class C3dAnimationSequence -//
//------------------------------//
C3dAnimationSequence::C3dAnimationSequence()
{
}

C3dAnimationSequence::C3dAnimationSequence(C3dAnimationSequence::Type type)
{
    m_type = type;
}

C3dAnimationSequence::Type C3dAnimationSequence::getType() const
{
    return m_type;
}

int C3dAnimationSequence::getFrameCount() const
{
    return m_moves.size();
}

void C3dAnimationSequence::generateMoves(int param)
{
    switch(m_type)
    {
        case ROTATE_UP:
            for(int angle=0; angle<param; ++angle)
            {
                C3dAnimationMove point;
                point.m_angle.setX(-1.0);
                m_moves.push_back(point);
            }
            break;

        case ROTATE_DOWN:
            for(int angle=0; angle<param; ++angle)
            {
                C3dAnimationMove point;
                point.m_angle.setX(1.0);
                m_moves.push_back(point);
            }
            break;

        case ROTATE_LEFT:
            for(int angle=0; angle<param; ++angle)
            {
                C3dAnimationMove point;
                point.m_angle.setY(-1.0);
                m_moves.push_back(point);
            }
            break;

        case ROTATE_RIGHT:
            for(int angle=0; angle<param; ++angle)
            {
                C3dAnimationMove point;
                point.m_angle.setY(1.0);
                m_moves.push_back(point);
            }
            break;

        case TRANSLATE_X:
            for(int i=0; i<param; ++i)
            {
                float x;
                param >= 0 ? x = 0.01f : x = -0.01f;
                C3dAnimationMove point;
                point.m_translate.setX(x);
                m_moves.push_back(point);
            }
            break;

        case TRANSLATE_Y:
            for(int i=0; i<param; ++i)
            {
                float y;
                param >= 0 ? y = 0.01f : y = -0.01f;
                C3dAnimationMove point;
                point.m_translate.setY(y);
                m_moves.push_back(point);
            }
            break;

        case ZOOM:
            for(int i=0; i<param; ++i)
            {
                float z;
                param >= 0 ? z = 0.01f : z = -0.01f;
                C3dAnimationMove point;
                point.m_translate.setZ(z);
                m_moves.push_back(point);
            }
            break;

        case FREE_MOVE: break;
    }
}

void C3dAnimationSequence::addRotation(const float &rx, const float &ry)
{
    C3dAnimationMove move;
    move.m_angle = QVector3D(rx, ry, 0.0);
    m_moves.push_back(move);
}

void C3dAnimationSequence::addTranslation(const float &dx, const float &dy, const float &dz)
{
    C3dAnimationMove move;
    move.m_translate = QVector3D(dx, dy, dz);
    m_moves.push_back(move);
}

void C3dAnimationSequence::clear()
{
    m_moves.clear();
}

QJsonObject C3dAnimationSequence::toJsonObject() const
{
    QJsonObject jsonSequence;
    jsonSequence["type"] = m_type;

    QJsonArray jsonRotations, jsonTranslations;
    for(int i=0; i<m_moves.size(); ++i)
    {
        QJsonObject jsonAngle;
        jsonAngle["x"] = m_moves[i].m_angle.x();
        jsonAngle["y"] = m_moves[i].m_angle.y();
        jsonRotations.append(jsonAngle);

        QJsonObject jsonTranslate;
        jsonTranslate["x"] = m_moves[i].m_translate.x();
        jsonTranslate["y"] = m_moves[i].m_translate.y();
        jsonTranslate["z"] = m_moves[i].m_translate.z();
        jsonTranslations.append(jsonTranslate);
    }
    jsonSequence["rotations"] = jsonRotations;
    jsonSequence["translations"] = jsonTranslations;
    return jsonSequence;
}

C3dAnimationSequence C3dAnimationSequence::fromJsonObject(QJsonObject jsonObj)
{
    Type type = static_cast<Type>(jsonObj["type"].toInt());
    C3dAnimationSequence sequence(type);
    QJsonArray jsonRotations = jsonObj["rotations"].toArray();
    QJsonArray jsonTranslations = jsonObj["translations"].toArray();
    assert(jsonRotations.size() == jsonTranslations.size());
    sequence.m_moves.clear();

    for(int i=0; i<jsonRotations.size(); ++i)
    {
        C3dAnimationMove move;
        QJsonObject jsonAngle = jsonRotations[i].toObject();
        move.m_angle.setX(jsonAngle["x"].toDouble());
        move.m_angle.setY(jsonAngle["y"].toDouble());

        QJsonObject jsonTranslate = jsonTranslations[i].toObject();
        move.m_translate.setX(jsonTranslate["x"].toDouble());
        move.m_translate.setY(jsonTranslate["y"].toDouble());
        move.m_translate.setZ(jsonTranslate["z"].toDouble());

        sequence.m_moves.push_back(move);
    }
    return sequence;
}

//----------------------//
//- Class C3dAnimation -//
//----------------------//
C3dAnimation::C3dAnimation()
{
}

void C3dAnimation::setFps(int fps)
{
    m_fps = fps;
}

int C3dAnimation::getFps() const
{
    return m_fps;
}

int C3dAnimation::getFrameCount() const
{
    int nb = 0;

    for(int i=0; i<m_sequences.size(); ++i)
        nb += m_sequences[i].getFrameCount();

    return nb;
}

void C3dAnimation::addSequence(const C3dAnimationSequence &sequence)
{
    m_sequences.push_back(sequence);
}

void C3dAnimation::addSequence(C3dAnimationSequence::Type type, int param)
{
    C3dAnimationSequence sequence(type);
    sequence.generateMoves(param);
    m_sequences.push_back(sequence);
}

void C3dAnimation::removeSequence(int index)
{
    if(index >= 0 && index < m_sequences.size())
        m_sequences.removeAt(index);
}

void C3dAnimation::clear()
{
    m_sequences.clear();
}

bool C3dAnimation::save(const QString &filePath)
{
    QJsonObject jsonAnim;
    jsonAnim["fps"] = m_fps;

    QJsonArray jsonSequences;
    for(int i=0; i<m_sequences.size(); ++i)
        jsonSequences.append(m_sequences[i].toJsonObject());

    jsonAnim["sequences"] = jsonSequences;

    QJsonDocument jsonDoc(jsonAnim);
    QFile jsonFile(filePath);

    if(jsonFile.open(QFile::WriteOnly) == false)
    {
        qCritical().noquote() << QObject::tr("The file %1 can't be created or opened in write mode").arg(filePath);
        return false;
    }

    jsonFile.write(jsonDoc.toJson());
    return true;
}

bool C3dAnimation::load(const QString &filePath)
{
    QFile jsonFile(filePath);
    if(jsonFile.open(QFile::ReadOnly) == false)
    {
        qCritical().noquote() << QObject::tr("The file %1 can't be opened").arg(filePath);
        return false;
    }

    m_sequences.clear();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll());
    if(jsonDoc.isNull())
    {
        qCritical().noquote() << QObject::tr("Invalid JSON data for this animation");
        return false;
    }

    QJsonObject jsonAnim = jsonDoc.object();
    if(jsonAnim.isEmpty())
    {
        qCritical().noquote() << QObject::tr("No data available for this animation");
        return false;
    }

    m_fps = jsonAnim["fps"].toInt();
    QJsonArray jsonSequences = jsonAnim["sequences"].toArray();

    for(int i=0; i<jsonSequences.size(); ++i)
        m_sequences.push_back(C3dAnimationSequence::fromJsonObject(jsonSequences[i].toObject()));

    return true;
}


