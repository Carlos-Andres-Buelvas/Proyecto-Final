#include "enemigo.h"
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QRandomGenerator>

Enemigo::Enemigo(float x, float y, float ancho, float alto)
    : Personaje(x, y, ancho, alto),
    direccion(1), frameActual(0), contador(0),
    velocidadAnimacion(10), estado(Patrullando)
{
    this->ancho *= 1.5;
    this->alto *= 1.5;
    cargarAnimaciones();

    setPixmap(framesDerecha[0].scaled(this->ancho, this->alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    setPos(x, 375 - alto + 170);
    posX = x;
    posY = 375 - alto;

    animacionTimer = new QTimer(this);
    connect(animacionTimer, &QTimer::timeout, this, &Enemigo::animarMovimiento);
    animacionTimer->start(80);
    velocidadMovimiento = 8;
}

void Enemigo::cargarAnimaciones() {
    QPixmap spriteSheet(":/sprites/Pictures/soldados.png");

    int w = 100;
    int h = 100;

    for (int i = 0; i < 3; ++i)
        framesIzquierda.append(spriteSheet.copy(i * w, 1 * h, w, h));

    for (int i = 0; i < 3; ++i)
        framesDerecha.append(spriteSheet.copy(i * w, 2 * h, w, h));
}

void Enemigo::animarMovimiento() {
    contador++;
    if (contador >= velocidadAnimacion) {
        QList<QPixmap>& frames = (direccion == -1) ? framesIzquierda : framesDerecha;
        setPixmap(frames[frameActual].scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        frameActual = (frameActual + 1) % frames.size();
        contador = 0;
    }

    if (estado == Patrullando) {
        posX += direccion * velocidadMovimiento;
        setX(posX);

        // Cambia de dirección si llega a bordes locales
        if (posX <= 800 || posX >= 1024)
            direccion *= -1;
    }
}

void Enemigo::mover() {
    // Movimiento constante con el scroll (lo llames desde Juego::actualizar)
    posX -= 4; // o usa velocidadScroll global si lo quieres dinámico
    setX(posX);
}

void Enemigo::disparar(QGraphicsScene* escena) {
    QGraphicsEllipseItem* disparo = new QGraphicsEllipseItem(0, 0, 15, 15);
    disparo->setBrush(Qt::red);
    disparo->setPos(x() - 10, y() + boundingRect().height() / 2 - 7);
    escena->addItem(disparo);
}

bool Enemigo::estaDisparando() const {
    return estado == Disparando;
}
