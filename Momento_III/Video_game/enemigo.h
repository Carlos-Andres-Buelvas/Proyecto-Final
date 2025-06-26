#ifndef ENEMIGO_H
#define ENEMIGO_H

#include "personaje.h"

class Enemigo : public Personaje {
    Q_OBJECT
public:
    Enemigo(float x, float y, float ancho, float alto);
    void mover() override;
};

#endif // ENEMIGO_H
