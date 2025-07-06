#include "enemigo.h"
#include "goku.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QTimer>
#include <QRandomGenerator>
#include <cmath>
#include <QDebug>

// Constructor del enemigo
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
    posY = y;

    // Inicia el disparo cuando el temporizador lo indique
    connect(timerDisparo, &QTimer::timeout, this, [=]() {
        if (scene()) {
            disparar(scene());
        }
    });

//NIVEL 2:

    // Dirección inicial aleatoria
    QStringList direcciones = { "arriba", "abajo", "izquierda", "derecha" };
    direccionActual = direcciones.at(QRandomGenerator::global()->bounded(4));

    // Timer para patrullar
    timerPatrulla = new QTimer(this);
    connect(timerPatrulla, &QTimer::timeout, this, &Enemigo::patrullar);
    timerPatrulla->start(100);  // cada 100 ms

}

// Cargar frames de la hoja de sprites
void Enemigo::cargarAnimaciones() {
    QPixmap spriteSheet(":/sprites/Pictures/soldados.png");

    int w = 100;
    int h = 100;

    for (int i = 0; i < 3; ++i)
        framesIzquierda.append(spriteSheet.copy(i * w, 1 * h, w, h));

    for (int i = 0; i < 3; ++i)
        framesDerecha.append(spriteSheet.copy(i * w, 2 * h, w, h));
}

// Movimiento general del enemigo (scroll, patrullaje y animación)
void Enemigo::mover() {
    posX -= 3;          // Efecto scroll
    setX(posX);

    // Detección de Goku para comenzar disparo
    if (gokuDetectado) {
        float distancia = std::abs(gokuDetectado->x() - x());

        if (distancia < 900 && gokuDetectado->x() < x()) {
            estado = Disparando;
            direccion = -1;

            if (!disparando) {
                disparando = true;
                timerDisparo->start(1000);  // Dispara cada 1 segundo
            }
        } else {
            estado = Patrullando;
            if (disparando) {
                disparando = false;
                timerDisparo->stop();
            }
        }
    }

    // Animación de caminar
    contador++;
    if (contador >= velocidadAnimacion) {
        QVector<QPixmap>& frames = (direccion == -1) ? framesIzquierda : framesDerecha;
        setPixmap(frames[frameActual].scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        frameActual = (frameActual + 1) % frames.size();
        contador = 0;
    }

    // Movimiento de patrullaje si no está disparando
    if (estado == Patrullando) {
        posX += direccion * velocidadMovimiento;
        setX(posX);

        if (posX <= 800 || posX >= 1300)
            direccion *= -1;
    }
}

// Disparo del enemigo
void Enemigo::disparar(QGraphicsScene* escena) {
    if (pausado || direccion != -1) return;  // Solo dispara si mira a la izquierda

    // Crear el proyectil rojo
    QGraphicsEllipseItem* proyectilSoldado = new QGraphicsEllipseItem(0, 0, 15, 15);
    proyectilSoldado->setBrush(Qt::red);
    proyectilSoldado->setZValue(1);

    float posDisparoY = y() + boundingRect().height() / 2 - 7;
    float posDisparoX = x() - 75;

    proyectilSoldado->setPos(posDisparoX, posDisparoY);
    escena->addItem(proyectilSoldado);

    // Movimiento del proyectil con temporizador
    QTimer* t = new QTimer(this);
    connect(t, &QTimer::timeout, [=]() mutable {
        if (!proyectilSoldado || !proyectilSoldado->scene()) {
            t->stop();
            t->deleteLater();
            return;
        }

        proyectilSoldado->moveBy(-20, 0);  // Se desplaza más rápido

        if (proyectilSoldado->x() < -100) {
            if (scene()) scene()->removeItem(proyectilSoldado);
            delete proyectilSoldado;

            // Eliminar de la lista de proyectiles activos
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

    // Registrar en la lista de proyectiles activos
    proyectilesActivos.append(qMakePair(proyectilSoldado, t));
}

// Elimina todos los proyectiles activos (cuando el enemigo muere)
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

// Pausa los temporizadores de los proyectiles
void Enemigo::detenerProyectiles() {
    for (auto& par : proyectilesActivos) {
        if (par.second) {
            par.second->stop();
        }
    }
}

// Reanuda el movimiento de proyectiles
void Enemigo::reanudarProyectiles() {
    for (auto& par : proyectilesActivos) {
        if (par.second) {
            par.second->start(30);
        }
    }
}

// Pausar o reanudar comportamiento completo del enemigo
void Enemigo::setPausado(bool pausa) {
    pausado = pausa;
    if (pausado) {
        if (timerDisparo && timerDisparo->isActive()) {
            timerDisparo->stop();
        }
        detenerProyectiles();
    } else {
        if (disparando && timerDisparo) {
            timerDisparo->start(1000);
        }
        reanudarProyectiles();
    }
}

//NIVEL 2:

void Enemigo::disparar2(QGraphicsScene* escena) {
    // implementación temporal vacía para compilar
}

void Enemigo::cargarAnimacionesNivel2(int tileW, int tileH) {
    QPixmap spriteSheet(":/sprites/Pictures/soldados.png");
    int frameW = 100;
    int frameH = 100;

    framesArriba.clear();
    framesAbajo.clear();
    framesIzquierda.clear();
    framesDerecha.clear();

    for (int i = 0; i < 3; ++i) {
        framesAbajo.append(spriteSheet.copy(i * frameW, 0 * frameH, frameW, frameH).scaled(tileW, tileH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        framesIzquierda.append(spriteSheet.copy(i * frameW, 1 * frameH, frameW, frameH).scaled(tileW, tileH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        framesDerecha.append(spriteSheet.copy(i * frameW, 2 * frameH, frameW, frameH).scaled(tileW, tileH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        framesArriba.append(spriteSheet.copy(i * frameW, 3 * frameH, frameW, frameH).scaled(tileW, tileH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    setPixmap(framesAbajo[0]);
}


void Enemigo::patrullar() {
    qDebug()<<"Patrullando";
    // Probar próxima posición sin moverse aún
    int paso = 4;
    QPointF nuevaPos = pos();

    if (direccionActual == "arriba") nuevaPos.ry() -= paso;
    else if (direccionActual == "abajo") nuevaPos.ry() += paso;
    else if (direccionActual == "izquierda") nuevaPos.rx() -= paso;
    else if (direccionActual == "derecha") nuevaPos.rx() += paso;

    // Si no choca con muro, avanzar
    if (!colisionaConMuro(nuevaPos, scene())) {
        mover2(direccionActual);
    } else {
        // Cambiar de dirección si choca
        QStringList nuevasDirecciones = { "arriba", "abajo", "izquierda", "derecha" };
        nuevasDirecciones.removeAll(direccionActual); // evitar repetir

        direccionActual = nuevasDirecciones.at(QRandomGenerator::global()->bounded(nuevasDirecciones.size()));
    }
}
