#ifndef ENEMIGO_H
#define ENEMIGO_H

#include "personaje.h"
#include <QList>
#include <QPixmap>
#include <QTimer>
#include <QGraphicsScene>

class Enemigo : public Personaje {
    Q_OBJECT
public:
    Enemigo(float x, float y, float ancho, float alto);

    void mover() override; // Se mueve junto al scroll
    void disparar(QGraphicsScene* escena) override;

    bool estaDisparando() const;

    enum EstadoEnemigo { Patrullando, Disparando, Muerto };
    EstadoEnemigo estado;

private:
    void cargarAnimaciones();
    void animarMovimiento();

    QList<QPixmap> framesIzquierda;
    QList<QPixmap> framesDerecha;

    int direccion;              // -1 = izquierda, 1 = derecha
    int frameActual;            // para saber en qué frame de animación va
    int contador;               // control de animación
    int velocidadAnimacion;     // cada cuántos ciclos cambiar frame

    QTimer* animacionTimer;     // para animar al soldado
    float velocidadMovimiento;
};

#endif // ENEMIGO_H
