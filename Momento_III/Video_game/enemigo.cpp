#include "enemigo.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QTimer>
#include <QRandomGenerator>
#include <cmath>

Enemigo::Enemigo(float x, float y, float ancho, float alto, Goku* gokuRef)
    : Personaje(x, y, ancho, alto),
    gokuDetectado(gokuRef),
    direccion(1),
    frameActual(0),
    contador(0),
    velocidadAnimacion(10),
    velocidadMovimiento(6),
    estado(Patrullando),
    disparando(false),
    timerDisparo(new QTimer(this))
{
    this->ancho *= 1.5;
    this->alto *= 1.5;

    cargarAnimaciones();
    setPixmap(framesDerecha[0].scaled(this->ancho, this->alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    posX = x;
    posY = 375 - alto + 170;
    setPos(posX, posY);

    animacionTimer = new QTimer(this);
    connect(animacionTimer, &QTimer::timeout, this, &Enemigo::animarMovimiento);
    animacionTimer->start(100);

    // conectar disparo al método heredado
    connect(timerDisparo, &QTimer::timeout, this, [=]() {
        if (scene()) {
            disparar(scene());  // usa método heredado correctamente
        }
    });
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
    if (!gokuDetectado) return;

    float distancia = std::abs(gokuDetectado->x() - x());

    // Solo dispara si Goku está cerca Y está a la izquierda del enemigo
    if (distancia < 800 && gokuDetectado->x() < x()) {
        estado = Disparando;
        direccion = -1;  // ← Fuerza dirección izquierda para animación y disparo

        if (!disparando) {
            disparando = true;
            timerDisparo->start(500);  // dispara cada medio segundo
        }
    } else {
        estado = Patrullando;
        if (disparando) {
            disparando = false;
            timerDisparo->stop();
        }
    }

    // Animar
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

        if (posX <= 800 || posX >= 1024)
            direccion *= -1;
    }
}

void Enemigo::mover() {
    posX -= 4; // para scroll
    setX(posX);
}

void Enemigo::disparar(QGraphicsScene* escena) {
    if (direccion != -1) return; // Solo permite disparo si va hacia la izquierda

    QGraphicsEllipseItem* proyectil = new QGraphicsEllipseItem(0, 0, 15, 15);
    proyectil->setBrush(Qt::red);
    proyectil->setZValue(1);

    float posDisparoY = y() + boundingRect().height() / 2 - 7;
    float posDisparoX = x() - 10;

    proyectil->setPos(posDisparoX, posDisparoY);
    escena->addItem(proyectil);

    QTimer* t = new QTimer(this);
    connect(t, &QTimer::timeout, [=]() mutable {
        if (!proyectil || !scene() || !scene()->items().contains(proyectil)) {
            if (proyectil) {
                scene()->removeItem(proyectil);
                delete proyectil;
            }
            t->stop();
            t->deleteLater();
            return;
        }

        proyectil->moveBy(-15, 0);

        if (proyectil->x() < -100) {
            scene()->removeItem(proyectil);
            delete proyectil;
            t->stop();
            t->deleteLater();
        }
    });

    t->start(30);
}

bool Enemigo::estaDisparando() const {
    return estado == Disparando;
}
