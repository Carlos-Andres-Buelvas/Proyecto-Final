#include "enemigo.h"

Enemigo::Enemigo(float x, float y, float ancho, float alto)
    : Personaje(x, y, ancho, alto) {
    setPixmap(QPixmap(":/sprites/Pictures/Shelom.png").scaled(ancho, alto));
}

void Enemigo::mover() {
    posX -= 5; // Se mueve a la izquierda
    setPos(posX, posY);
}
