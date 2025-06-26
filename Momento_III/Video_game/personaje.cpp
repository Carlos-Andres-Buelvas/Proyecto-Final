#include "personaje.h"

Personaje::Personaje(float x, float y, float ancho, float alto)
    : posX(x), posY(y), velocidadY(0), gravedad(0.9), ancho(ancho), alto(alto) {
    setPos(posX, posY);
}
