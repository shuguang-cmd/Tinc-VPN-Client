#ifndef ANIMATEDCHARACTER_H
#define ANIMATEDCHARACTER_H

#include <QWidget>
#include <QPoint>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QTimer>

class AnimatedCharacter : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(float handOffset READ handOffset WRITE setHandOffset)
    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor)

public:
    explicit AnimatedCharacter(QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedSize(160, 100);
        m_animation = new QPropertyAnimation(this, "handOffset", this);
        m_animation->setDuration(300);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);

        m_bodyColor = QColor("#764ba2");
        m_colorAnimation = new QPropertyAnimation(this, "bodyColor", this);
        m_colorAnimation->setDuration(4000);
        m_colorAnimation->setStartValue(QColor("#764ba2")); // Blue-Purple
        m_colorAnimation->setKeyValueAt(0.5, QColor("#d4af37")); // Gold
        m_colorAnimation->setEndValue(QColor("#1a1a1a")); // Black
        m_colorAnimation->setLoopCount(-1);
        m_colorAnimation->start();
    }

    void setTargetPos(const QPoint& pos) {
        m_targetPos = mapFromGlobal(pos);
        update();
    }

    void setCoverEyes(bool cover) {
        if (m_coverEyes == cover) return;
        m_coverEyes = cover;
        m_animation->stop();
        m_animation->setStartValue(m_handOffset);
        m_animation->setEndValue(cover ? 1.0f : 0.0f);
        m_animation->start();
    }

    float handOffset() const { return m_handOffset; }
    void setHandOffset(float offset) { m_handOffset = offset; update(); }

    QColor bodyColor() const { return m_bodyColor; }
    void setBodyColor(const QColor& color) { m_bodyColor = color; update(); }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int centerX = width() / 2;
        int centerY = height() / 2 + 10;

        // Draw Head with dynamic color
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_bodyColor);
        painter.drawRoundedRect(centerX - 50, centerY - 45, 100, 90, 45, 45);

        // Draw Ears
        painter.drawEllipse(centerX - 55, centerY - 50, 30, 30);
        painter.drawEllipse(centerX + 25, centerY - 50, 30, 30);

        // Draw Eyes
        drawEye(&painter, centerX - 25, centerY - 10);
        drawEye(&painter, centerX + 25, centerY - 10);

        // Draw Hands
        drawHands(&painter, centerX, centerY);
    }

private:
    void drawEye(QPainter *painter, int x, int y) {
        // Sclera
        painter->setBrush(Qt::white);
        painter->drawEllipse(x - 12, y - 12, 24, 24);

        // Pupil movement calculation
        QPoint eyeCenter(x, y);
        QPoint direction = m_targetPos - eyeCenter;
        float dist = qSqrt(direction.x() * direction.x() + direction.y() * direction.y());
        
        float maxOffset = 6.0f;
        QPointF pupilOffset(0, 0);
        if (dist > 0.1f) {
            float moveDist = qMin(dist / 10.0f, maxOffset);
            pupilOffset.setX(direction.x() / dist * moveDist);
            pupilOffset.setY(direction.y() / dist * moveDist);
        }

        // Pupil
        painter->setBrush(QColor("#333333"));
        painter->drawEllipse(QRectF(x - 5 + pupilOffset.x(), y - 5 + pupilOffset.y(), 10, 10));
    }

    void drawHands(QPainter *painter, int centerX, int centerY) {
        painter->setBrush(m_bodyColor);
        
        float offset = m_handOffset * 40.0f; // Hand movement range
        
        // Left hand
        painter->drawRoundedRect(centerX - 55, centerY + 30 - offset, 35, 40, 15, 15);
        // Right hand
        painter->drawRoundedRect(centerX + 20, centerY + 30 - offset, 35, 40, 15, 15);
    }

    QPoint m_targetPos;
    bool m_coverEyes = false;
    float m_handOffset = 0.0f;
    QColor m_bodyColor;
    QPropertyAnimation *m_animation;
    QPropertyAnimation *m_colorAnimation;
};

#endif // ANIMATEDCHARACTER_H
