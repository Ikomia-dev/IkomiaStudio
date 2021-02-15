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

#ifndef CGLWIGDET_H
#define CGLWIGDET_H

/**
 * @file      CGLWidget.h
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Header file including CGLWidget definition
 *
 * @details   Details
 */
#include <QtOpenGL>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "Main/forwards.hpp"
#include "Model/Render/C3dAnimation.h"
#include <QOpenGLFunctions>

class CDataVideoBuffer;

constexpr double pi = 3.14159265358979323846;

/**
 * @brief The CGLWidget class initializes an OpenGL widget for dispslaying 3D view
 *
 */
class CGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

    public:

        /**
         * @brief
         *
         * @param parent
         */
        CGLWidget(QWidget *parent = Q_NULLPTR);
        /**
         * @brief
         *
         */
        ~CGLWidget();

        void        init();
        void        initGL();

        QVector3D   getRotationVector() const;
        QVector3D   getTranslateVector() const;

        void        exportScreenshot(const QString& path);

        void        startAnimationRecording();
        void        stopAnimationRecording();
        void        playAnimation(const C3dAnimation& animation);
        void        makeVideoAnimation(const QString& path, const C3dAnimation& animation);

        void        activateRender(bool bEnable);

    signals:

        void        doInitCL(const std::string& vendorName);
        void        doInitPBO(GLuint pbo, GLuint pbo_depth);
        void        doRenderVolume(QMatrix4x4& model, QMatrix4x4& view, QMatrix4x4& projection);
        void        doUpdateWindowSize(int width, int height);
        void        doAddAnimationSequence(const C3dAnimationSequence& sequence);
        void        doSendErrorMessage(const QString& msg);

    private slots:

        void        onUpdateAnimation();
        void        onAddAnimationVideoFrame();

    protected:

        void        initCamera();
        void        initCameraPos();
        void        initLight();
        void        initCL();
        void        initPBO();

        void        setProjection(int w, int h);
        void        setView(int w, int h);

        void        updateModelMatrix();

        void        makeScene();

        void        displayTexture();
        void        displayAxes();

        void        initializeGL() Q_DECL_OVERRIDE;
        void        paintGL() Q_DECL_OVERRIDE;
        void        resizeGL(int w, int h) Q_DECL_OVERRIDE;
        void        mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
        void        mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
        void        wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;

    private:

        QMatrix4x4              m_projectionMat;
        QMatrix4x4              m_viewMat;
        QMatrix4x4              m_modelMat;
        QVector3D               m_eye;
        QVector3D               m_center;
        QVector3D               m_up;
        QVector3D               m_eulerAngle;
        QVector3D               m_modelTranslate;
        QQuaternion             m_modelQuat;
        QPointF                 m_mousePressPos;
        double                  m_zNear;
        double                  m_zFar;
        float                   m_fov;
        int                     m_wndWidth;
        int                     m_wndHeight;
        int                     m_frames;
        GLuint                  m_pbo = 0;
        GLuint                  m_pbo_depth = 0;
        GLuint                  m_texture = 0;
        GLuint                  m_texture_depth = 0;
        QTime                   m_time;
        //Animation
        C3dAnimation            m_animation;
        C3dAnimationSequence    m_animationSequence;
        CDataVideoBuffer*       m_pVideoWriter = nullptr;
        QTimer*                 m_pAnimationTimer = nullptr;
        bool                    m_bAnimationRecording = false;
        int                     m_currentAnimationSequence = 0;
        int                     m_currentAnimationMove = 0;
        bool                    m_bUseShader = false;
        bool                    m_bEnableRender = false;
};

#endif // CGLWIGDET_H
