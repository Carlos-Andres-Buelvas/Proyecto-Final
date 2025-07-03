#include "enemigo.h"
#include "goku.h"
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
    velocidadAnimacion(50),
    velocidadMovimiento(1),
    estado(Patrullando),
    disparando(false),
    timerDisparo(new QTimer(this))
{
    this->ancho *= 1.5;
    this->alto *= 1.5;

    cargarAnimaciones();
    setPixmap(framesDerecha[0].scaled(this->ancho, this->alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    posX = x;
    posY = y; //375 - alto + 170;
    //setPos(posX, posY);

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

void Enemigo::mover() {
    // Desplazamiento general por scroll
    posX -= 3;
    setX(posX);

    // 🎯 Detección de Goku
    if (gokuDetectado) {
        float distancia = std::abs(gokuDetectado->x() - x());

        if (distancia < 950 && gokuDetectado->x() < x()) {
            estado = Disparando;
            direccion = -1;

            if (!disparando) {
                disparando = true;
                timerDisparo->start(1000);  // dispara cada 1s
            }
        } else {
            estado = Patrullando;
            if (disparando) {
                disparando = false;
                timerDisparo->stop();
            }
        }
    }

    // 🎞️ Animación de frames
    contador++;
    if (contador >= velocidadAnimacion) {
        QVector<QPixmap>& frames = (direccion == -1) ? framesIzquierda : framesDerecha;
        setPixmap(frames[frameActual].scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        frameActual = (frameActual + 1) % frames.size();
        contador = 0;
    }

    // 🚶‍♂️ Movimiento lateral si está patrullando
    if (estado == Patrullando) {
        posX += direccion * velocidadMovimiento;
        setX(posX);

        if (posX <= 800 || posX >= 1300)
            direccion *= -1;
    }
}

void Enemigo::disparar(QGraphicsScene* escena) {
    if (direccion != -1) return; // Solo dispara si está mirando a la izquierda

    // Crear el proyectil visual
    QGraphicsEllipseItem* proyectilSoldado = new QGraphicsEllipseItem(0, 0, 15, 15);
    proyectilSoldado->setBrush(Qt::red);
    proyectilSoldado->setZValue(1);

    float posDisparoY = y() + boundingRect().height() / 2 - 7;
    float posDisparoX = x() - 75;

    proyectilSoldado->setPos(posDisparoX, posDisparoY);
    escena->addItem(proyectilSoldado);

    // Timer para mover el proyectil
    QTimer* t = new QTimer(this);
    connect(t, &QTimer::timeout, [=]() mutable {
        if (!proyectilSoldado || !proyectilSoldado->scene()) {
            t->stop();
            t->deleteLater();
            return;
        }

        proyectilSoldado->moveBy(-20, 0); // más rápido

        if (proyectilSoldado->x() < -100) {
            if (scene()) scene()->removeItem(proyectilSoldado);
            delete proyectilSoldado;

            // Limpiar de la lista
            for (int i = 0; i < proyectilesActivos.size(); ++i) {
                if (proyectilesActivos[i].first == proyectilSoldado) {
                    QTimer* tmp = proyectilesActivos[i].second;
                    if (tmp) { tmp->stop(); tmp->deleteLater(); }
                    proyectilesActivos.removeAt(i);
                    break;
                }
            }

            t->stop();
            t->deleteLater();
        }
    });
    t->start(30);

    // Guardar para eliminar después
    proyectilesActivos.append(qMakePair(proyectilSoldado, t));
/*
    // **AÑADE ESTO**: reprográmate a ti mismo dentro de 5 s
    QTimer::singleShot(10000, this, [=]() {
        disparar(escena);
    });
*/
}

bool Enemigo::estaDisparando() const {
    return estado == Disparando;
}

void Enemigo::eliminarProyectiles() {
    for (const auto& par : proyectilesActivos) {
        auto proyectil = par.first;
        auto timer = par.second;

        if (scene()) scene()->removeItem(proyectil);
        delete proyectil;

        if (timer) {
            timer->stop();
            timer->deleteLater();
        }
    }
    proyectilesActivos.clear();
}

