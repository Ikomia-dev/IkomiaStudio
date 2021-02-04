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
