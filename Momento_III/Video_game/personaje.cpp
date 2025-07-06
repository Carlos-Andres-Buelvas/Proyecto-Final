#include "personaje.h"
#include <QGraphicsScene>

// Constructor base para cualquier personaje
Personaje::Personaje(float x, float y, float ancho, float alto)
    : posX(x), posY(y), velocidadY(0), gravedad(1), ancho(ancho), alto(alto)
{
    setPos(posX, posY);  // Establece la posición inicial en la escena
}

//NIVEL 2:

void Personaje::mover2(const QString& direccion) {
    int paso = 4;
    QPointF nuevaPos = pos();  // posición actual

    if (direccion == "arriba") nuevaPos.ry() -= paso;
    else if (direccion == "abajo") nuevaPos.ry() += paso;
    else if (direccion == "izquierda") nuevaPos.rx() -= paso;
    else if (direccion == "derecha") nuevaPos.rx() += paso;

    // Verificar colisión con muros
    if (!colisionaConMuro(nuevaPos, scene())) {
        setPos(nuevaPos);  // Solo si no choca
        posX = nuevaPos.x();
        posY = nuevaPos.y();
    }

    // Animación
    if (direccion == "arriba")
        setPixmap(framesArriba[frameActual++ % framesArriba.size()]);
    else if (direccion == "abajo")
        setPixmap(framesAbajo[frameActual++ % framesAbajo.size()]);
    else if (direccion == "izquierda")
        setPixmap(framesIzquierda[frameActual++ % framesIzquierda.size()]);
    else if (direccion == "derecha")
        setPixmap(framesDerecha[frameActual++ % framesDerecha.size()]);
}

bool Personaje::colisionaConMuro(const QPointF& nuevaPos, QGraphicsScene* escena) {
    // Creamos un rectángulo en la nueva posición que simula la hitbox de Goku
    int margen = 6; // Puedes ajustar: 2–6 según sensibilidad deseada
    QRectF rectNuevo = QRectF(
        nuevaPos.x() + margen,
        nuevaPos.y() + margen,
        boundingRect().width() - 2 * margen,
        boundingRect().height() - 2 * margen
        );

    // Verificamos si colisionaría con algún muro
    QList<QGraphicsItem*> items = escena->items(rectNuevo);

    for (QGraphicsItem* item : items) {
        if (item != this && item->data(0) == "muro") {
            return true;
        }
    }

    return false;
}
