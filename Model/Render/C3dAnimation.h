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

#ifndef C3DANIMATION_H
#define C3DANIMATION_H

#include <QVector3D>

struct C3dAnimationMove
{
    public:

        QVector3D   m_angle;
        QVector3D   m_translate;
};

class C3dAnimationSequence
{
    public:

        enum Type { ROTATE_UP, ROTATE_DOWN, ROTATE_LEFT, ROTATE_RIGHT, TRANSLATE_X, TRANSLATE_Y, ZOOM, FREE_MOVE};

        C3dAnimationSequence();
        C3dAnimationSequence(Type type);

        Type        getType() const;
        int         getFrameCount() const;

        void        generateMoves(int param);

        void        addRotation(const float& rx, const float& ry);
        void        addTranslation(const float& dx, const float& dy, const float &dz);

        void        clear();

        QJsonObject toJsonObject() const;

        static C3dAnimationSequence fromJsonObject(QJsonObject jsonObj);

    public:

        QVector<C3dAnimationMove>   m_moves;

    private:

        Type                        m_type = FREE_MOVE;
};

class C3dAnimation
{
    public:

        C3dAnimation();

        void    setFps(int fps);

        int     getFps() const;
        int     getFrameCount() const;

        void    addSequence(const C3dAnimationSequence& sequence);
        void    addSequence(C3dAnimationSequence::Type type, int param);

        void    removeSequence(int index);

        void    clear();

        bool    save(const QString& filePath);
        bool    load(const QString& filePath);

    public:

        QVector<C3dAnimationSequence>   m_sequences;

    private:

        int                             m_fps = 30;
};

#endif // C3DANIMATION_H
