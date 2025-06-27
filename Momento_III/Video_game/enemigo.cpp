#include "enemigo.h"
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>

Enemigo::Enemigo(float x, float y, float ancho, float alto)
    : Personaje(x, y, ancho, alto) {
    setPixmap(QPixmap(":/sprites/Pictures/Shelom.png").scaled(ancho, alto));
}

void Enemigo::mover() {
    setX(x() - 2);  // Enemigo se mueve hacia la izquierda
}

void Enemigo::disparar(QGraphicsScene* escena) {
    QGraphicsEllipseItem* disparo = new QGraphicsEllipseItem(0, 0, 15, 15);
    disparo->setBrush(Qt::red); // Disparo rojo
    disparo->setPos(x() - 10, y() + boundingRect().height()/2 - 7);
    escena->addItem(disparo);
}
