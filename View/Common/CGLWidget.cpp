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

/**
 * @file      CGLWidget.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CGLWidget
 *
 * @details   Details
 */

#include "CGLWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include "CAxisRender.h"
#include "CTextureRender.h"
#include "Data/CMat.hpp"
#include "CDataVideoBuffer.h"
#include "View/Main/CMainView.h"
#include "VolumeRender.h"
#include "Main/LogCategory.h"

CGLWidget::CGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    m_projectionMat.setToIdentity();
    m_viewMat.setToIdentity();
    m_modelMat.setToIdentity();

    m_mousePressPos = QPointF(-1.0, -1.0);
    m_fov = 45.0;
    m_frames = 0;
}

CGLWidget::~CGLWidget()
{
    doneCurrent();

    if(m_pVideoWriter)
        delete m_pVideoWriter;
}

void CGLWidget::initCamera()
{
    m_center = QVector3D(0, 0, 0);
    m_up = QVector3D(0, 1, 0);
    m_viewMat.setToIdentity();
    m_modelQuat = QQuaternion(1.0, 0.0, 0.0, 0.0);
    m_modelTranslate = QVector3D(0.0, 0.0, 0.0);
    m_eulerAngle = QVector3D(0.0, 0.0, 0.0);
    m_fov = 45.0;
    m_zNear = 0.1f;
}

void CGLWidget::initCameraPos()
{
    setView(m_wndWidth, m_wndHeight);
}

void CGLWidget::initLight()
{
    GLfloat lightModel[] = {GL_TRUE};
    glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lightModel);

    //Lumière pour la scène principale
    GLfloat lightAmbient0[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse0);
}

void CGLWidget::initCL()
{
    auto ctx = context();
    // Print OpenGL information
    if(ctx == 0 || !ctx->isValid())
    {
        qCWarning(logRender()).noquote() << tr("Null or invalid OpenGL context.");
        emit doSendErrorMessage(tr("Please verify your OpenGL installation. There is currently no available OpenGL context."));
        return;
    }

    int majVer = context()->format().majorVersion();
    int minVer = context()->format().minorVersion();
    auto vendorName = QString((const char*)glGetString(GL_VENDOR));
    qDebug(logRender()) << "Really used OpenGl: " << majVer << "." << minVer;
    qDebug(logRender()) << "OpenGL information: VENDOR:       " << vendorName;
    qDebug(logRender()) << "                    RENDERER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug(logRender()) << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug(logRender()) << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    // Test OpenGL version
    if(majVer>3)
        m_bUseShader = true;
    else if(majVer==3 && minVer>=3)
        m_bUseShader = true;
    else
        m_bUseShader = false;

    // Check OpenGL driver vendor
    std::string vendor;
    if(vendorName.contains("NVIDIA", Qt::CaseInsensitive))
        vendor = NVIDIA_PLATFORM;
    else if(vendorName.contains("ATI", Qt::CaseInsensitive) || vendorName.contains("AMD", Qt::CaseInsensitive))
        vendor = AMD_PLATFORM;
    else if(vendorName.contains("INTEL", Qt::CaseInsensitive))
        vendor = INTEL_PLATFORM;
    else if(vendorName.contains("MESA", Qt::CaseInsensitive) || vendorName.contains("X.Org", Qt::CaseInsensitive))
        vendor = MESA_PLATFORM;
    else if(vendorName.contains("APPLE", Qt::CaseInsensitive))
        vendor = APPLE_PLATFORM;

    // Make GL context current for CL-GL interop
    makeCurrent();
    emit doInitCL(vendor);
}

void CGLWidget::initPBO()
{
    if (m_pbo != 0)
    {
        // delete old buffer
        glDeleteBuffers(1, &m_pbo);
        m_pbo = 0;
    }

    if (m_pbo_depth != 0)
    {
        glDeleteBuffers(1, &m_pbo_depth);
        m_pbo_depth = 0;
    }

    // create pixel buffer object (PBO) for depth writing
    glGenBuffers(1, &m_pbo_depth);
    glBindBuffer(GL_ARRAY_BUFFER, m_pbo_depth);
    glBufferData(GL_ARRAY_BUFFER, m_wndWidth*m_wndHeight*sizeof(GLfloat)*4, 0, GL_DYNAMIC_DRAW);
    // Unset PBO as current object
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // create pixel buffer object (PBO) for display
    glGenBuffers(1, &m_pbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_pbo);
    glBufferData(GL_ARRAY_BUFFER, m_wndWidth*m_wndHeight*sizeof(GLubyte)*4, 0, GL_DYNAMIC_DRAW);
    // Unset PBO as current object
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    emit doInitPBO(m_pbo, m_pbo_depth);
}

void CGLWidget::init()
{
    initCamera();
    initCameraPos();
    m_modelMat.setToIdentity();
    m_mousePressPos = QPointF(-1.0, -1.0);
    m_frames = 0;
    initPBO();
    setProjection(m_wndWidth, m_wndHeight);
    emit doUpdateWindowSize(m_wndWidth, m_wndHeight);
}

void CGLWidget::initGL()
{
    if(!m_bEnableRender)
        initCL();

    initCamera();
    initLight();
}

QVector3D CGLWidget::getRotationVector() const
{
    return m_eulerAngle;
}

QVector3D CGLWidget::getTranslateVector() const
{
    return m_modelTranslate;
}

void CGLWidget::exportScreenshot(const QString &path)
{
    makeCurrent();
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    unsigned char* pBuffer = new unsigned char[m_wndWidth*m_wndHeight*3];
    glReadPixels(0, 0, m_wndWidth, m_wndHeight, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);
    CMat screen(m_wndHeight, m_wndWidth, CV_8UC3, pBuffer);
    cv::flip(screen, screen, 0);
    cv::imwrite(path.toStdString(), screen);
    delete[] pBuffer;
}

void CGLWidget::startAnimationRecording()
{
    m_bAnimationRecording = true;
}

void CGLWidget::stopAnimationRecording()
{
    m_bAnimationRecording = false;
    emit doAddAnimationSequence(m_animationSequence);
    m_animationSequence.clear();
}

void CGLWidget::playAnimation(const C3dAnimation &animation)
{
    if(animation.m_sequences.size() == 0)
        return;

    if(m_pAnimationTimer)
        return;

    m_pAnimationTimer = new QTimer(this);
    connect(m_pAnimationTimer, &QTimer::timeout, this, &CGLWidget::onUpdateAnimation);
    m_animation = animation;
    m_currentAnimationSequence = 0;
    m_currentAnimationMove = 0;
    m_pAnimationTimer->start(1000 / m_animation.getFps());
}

void CGLWidget::makeVideoAnimation(const QString &path, const C3dAnimation &animation)
{
    if(animation.m_sequences.size() == 0)
        return;

    if(m_pAnimationTimer)
        return;

    if(m_pVideoWriter)
        delete m_pVideoWriter;

    m_animation = animation;
    m_pVideoWriter = new CDataVideoBuffer(path.toStdString());
    //m_pVideoWriter->setFrameCount(m_animation.getFrameCount());
    m_pVideoWriter->startStreamWrite(m_wndWidth, m_wndHeight, m_animation.getFps());

    m_pAnimationTimer = new QTimer(this);
    connect(m_pAnimationTimer, &QTimer::timeout, this, &CGLWidget::onAddAnimationVideoFrame);
    m_currentAnimationSequence = 0;
    m_currentAnimationMove = 0;
    m_pAnimationTimer->start(1000 / m_animation.getFps());
}

void CGLWidget::activateRender(bool bEnable)
{
    m_bEnableRender = bEnable;
    if(bEnable == false)
       emit doSendErrorMessage(tr("GPU initialization failed. Please see the log file for more details."));
}

void CGLWidget::onUpdateAnimation()
{
    if(m_currentAnimationMove >= m_animation.m_sequences[m_currentAnimationSequence].m_moves.size())
    {
        m_currentAnimationSequence++;
        m_currentAnimationMove = 0;
    }

    if(m_currentAnimationSequence >= m_animation.m_sequences.size())
    {
        m_pAnimationTimer->stop();
        delete m_pAnimationTimer;
        m_pAnimationTimer = nullptr;
        return;
    }

    m_eulerAngle += m_animation.m_sequences[m_currentAnimationSequence].m_moves[m_currentAnimationMove].m_angle;
    m_modelTranslate += m_animation.m_sequences[m_currentAnimationSequence].m_moves[m_currentAnimationMove].m_translate;
    m_currentAnimationMove++;
    update();
}

void CGLWidget::onAddAnimationVideoFrame()
{
    if(m_currentAnimationMove >= m_animation.m_sequences[m_currentAnimationSequence].m_moves.size())
    {
        m_currentAnimationSequence++;
        m_currentAnimationMove = 0;
    }

    if(m_currentAnimationSequence >= m_animation.m_sequences.size())
    {
        m_pVideoWriter->stopWrite(0);
        m_pAnimationTimer->stop();
        delete m_pAnimationTimer;
        m_pAnimationTimer = nullptr;
        return;
    }

    m_eulerAngle += m_animation.m_sequences[m_currentAnimationSequence].m_moves[m_currentAnimationMove].m_angle;
    m_modelTranslate += m_animation.m_sequences[m_currentAnimationSequence].m_moves[m_currentAnimationMove].m_translate;
    m_currentAnimationMove++;
    update();

    makeCurrent();
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    unsigned char* pBuffer = new unsigned char[m_wndWidth*m_wndHeight*3];
    glReadPixels(0, 0, m_wndWidth, m_wndHeight, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);
    CMat screen(m_wndHeight, m_wndWidth, CV_8UC3, pBuffer);
    cv::flip(screen, screen, 0);
    m_pVideoWriter->write(screen);
    delete[] pBuffer;
}

void CGLWidget::updateModelMatrix()
{
    QQuaternion quat = QQuaternion::fromEulerAngles(m_eulerAngle);
    m_modelQuat = quat * m_modelQuat;
    m_modelMat.setToIdentity();
    m_modelMat.translate(m_modelTranslate);
    m_modelMat.rotate(m_modelQuat);
    m_eulerAngle.setX(0);
    m_eulerAngle.setY(0);
}

void CGLWidget::setProjection(int w, int h)
{
    //m_fov = 45.0*m_zoomFactor;
    m_projectionMat.setToIdentity();
    m_projectionMat.perspective(m_fov, (float)w/(float)h, m_zNear, m_zFar);
}

void CGLWidget::setView(int w, int h)
{
    float sizeView = (float)w/(float)h;
    double eyeZ = 1.5f * (w / std::max<float>((float)w, (float)h)) / std::tan((m_fov/2.0f)*pi/180.0f);

    if(sizeView<1)
        eyeZ /= sizeView;
    else
        eyeZ *= sizeView;

    m_eye = QVector3D(0, 0, eyeZ);
    m_viewMat.setToIdentity();
    m_viewMat.lookAt(m_eye, m_center, m_up);
    m_zFar = 10.0f * eyeZ;
}

void CGLWidget::makeScene()
{
    if(!m_bEnableRender)
        return;

    //OpenCL rendering -> fill PBO
    emit doRenderVolume(m_modelMat, m_viewMat, m_projectionMat);

    try
    {
        // Display PBO as texture
        displayTexture();

        if(m_bUseShader)
        {
            //Display orthogonal axes
            displayAxes();
        }
    }
    catch(CException& e)
    {
        qDebug(logRender()) << e.what();
    }
}

void CGLWidget::displayTexture()
{
    glDisable(GL_DEPTH_TEST);
    CTextureRender textureRender;
    textureRender.setSizes(m_wndWidth, m_wndHeight);
    textureRender.setMatrix(m_modelMat, m_viewMat, m_projectionMat);
    textureRender.setPBO(m_pbo);
    textureRender.setShaderMode(m_bUseShader);
    textureRender.render();
}

void CGLWidget::displayAxes()
{
    glEnable(GL_DEPTH_TEST);
    QMatrix4x4 modelMat;
    modelMat.setToIdentity();
    modelMat.rotate(m_modelQuat);

    QMatrix4x4 viewMat;
    viewMat.setToIdentity();
    viewMat.lookAt(QVector3D(0.0, 0.0, 3.0), QVector3D(0.0, 0.0, 0.0), QVector3D(0.0, 1.0, 0.0));

    QMatrix4x4 projMat;
    projMat.setToIdentity();
    projMat.perspective(m_fov, 1.0, m_zNear, m_zFar);

    int size = (int)(0.1 * m_wndWidth);
    glViewport(0, 0, size, size);
    CAxisRender axis(&modelMat, &viewMat, &projMat, size, size, m_fov);
    axis.render();
}

void CGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
}

void CGLWidget::resizeGL(int w, int h)
{
    // Get pixel ratio mainly for retina display on Mac
    int pxRatio = devicePixelRatio();
    m_wndWidth = w*pxRatio;
    m_wndHeight = h*pxRatio;
    initPBO();
    //setView(w, h);
    //setProjection(w, h);
    emit doUpdateWindowSize(m_wndWidth, m_wndHeight);
}

void CGLWidget::paintGL()
{
    /*QPainter painter;
    painter.begin(this);
    painter.beginNativePainting();*/

    glViewport(0, 0, m_wndWidth, m_wndHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Writing the RGBA buffer
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    //glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);
    glFinish();

    updateModelMatrix();
    makeScene();

    //painter.endNativePainting();

    /*if (const int elapsed = m_time.elapsed())
    {
        QString framesPerSecond;
        framesPerSecond.setNum(m_frames /(elapsed / 1000.0), 'f', 2);
        painter.setPen(Qt::white);
        painter.drawText(10, 20, framesPerSecond + " paintGL calls / s");
        painter.drawRoundRect(5, 5, 150, 20, 25, 25);
    }*/

    if (!(m_frames % 100))
    {
        m_time.start();
        m_frames = 0;
    }
    ++m_frames;
}

void CGLWidget::mousePressEvent(QMouseEvent *event)
{
    m_mousePressPos = event->localPos();
    if(event->buttons() == Qt::MiddleButton)
    {
        m_modelMat.setToIdentity();
        initCamera();
        initCameraPos();
        update();
    }
    QWidget::mousePressEvent(event);
}

void CGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(m_mousePressPos.x() >= 0 && m_mousePressPos.y() >= 0)
    {
        if(event->buttons() == Qt::LeftButton)
        {
            float rx = 0.5f * (event->localPos().y() - m_mousePressPos.y());
            float ry = 0.5f * (event->localPos().x() - m_mousePressPos.x());
            m_eulerAngle.setX(rx);
            m_eulerAngle.setY(ry);

            if(m_bAnimationRecording == true)
                m_animationSequence.addRotation(rx, ry);
        }
        else if(event->buttons() == Qt::RightButton)
        {
            float dx =  0.01 * (event->localPos().x() - m_mousePressPos.x());
            float dy =  0.01 * (event->localPos().y() - m_mousePressPos.y());
            m_modelTranslate.setX(m_modelTranslate.x() + dx);
            m_modelTranslate.setY(m_modelTranslate.y() - dy);

            if(m_bAnimationRecording == true)
                m_animationSequence.addTranslation(dx, -dy, 0.0);
        }
        m_mousePressPos = event->localPos();
        update();
    }
}

void CGLWidget::wheelEvent(QWheelEvent *event)
{
    float dz = 0.1 * event->delta() / 120.0;
    m_modelTranslate.setZ(m_modelTranslate.z() + dz);
    update();

    if(m_bAnimationRecording == true)
        m_animationSequence.addTranslation(0.0, 0.0, dz);
}


