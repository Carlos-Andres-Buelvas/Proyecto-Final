#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QGraphicsPixmapItem>
#include <QTimer>

class Personaje : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    Personaje(float x, float y, float ancho, float alto);
    virtual void mover() = 0;
    virtual void disparar(QGraphicsScene* escena) = 0;


protected:
    float posX, posY;
    float velocidadY;
    float gravedad;
    float ancho, alto;
};

#endif // PERSONAJE_H
