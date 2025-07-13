#include "enemigo.h"
#include "goku.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QTimer>
#include <QRandomGenerator>
#include <cmath>
#include <QDebug>

/**
 * @brief Constructor del Enemigo.
 * Inicializa el personaje dependiendo si está en nivel 1 o nivel 2.
 * En el nivel 1 carga animaciones y temporizador de disparo.
 * En el nivel 2 configura dirección aleatoria y lógica de patrullaje/disparo.
 */
Enemigo::Enemigo(float x, float y, float ancho, float alto, Goku* gokuRef, bool esNivel2)
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
    if (!esNivel2) {
        this->ancho *= 1.5;
        this->alto *= 1.5;
        cargarAnimaciones();
        setPixmap(framesDerecha[0].scaled(this->ancho, this->alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        posX = x;
        posY = y;
        connect(timerDisparo, &QTimer::timeout, this, [=]() {
            if (scene()) {
                disparar(scene());
            }
        });
    } else {
        tipo = "enemigo";
        goku = gokuRef;
        QStringList direcciones = { "arriba", "abajo", "izquierda", "derecha" };
        direccionActual = direcciones.at(QRandomGenerator::global()->bounded(4));
        timerPatrulla = new QTimer(this);
        connect(timerPatrulla, &QTimer::timeout, this, &Enemigo::patrullar);
        timerPatrulla->start(100);

        timerDisparo2 = new QTimer(this);
        timerDisparo2->setInterval(tiempoEsperaDisparo);
        connect(timerDisparo2, &QTimer::timeout, this, [this]() {
            if (this->goku && this->scene()){
                qreal distancia = QLineF(this->pos(), this->goku->pos()).length();
                if (distancia < 250) {
                    this->disparar2(scene());
                }
            }
        });
        timerDisparo2->start();
    }
}

/**
 * @brief Carga animaciones del enemigo para nivel 1 desde la hoja de sprites.
 */
void Enemigo::cargarAnimaciones() {
    QPixmap spriteSheet(":/sprites/Pictures/soldados.png");
    int w = 100;
    int h = 100;
    for (int i = 0; i < 3; ++i)
        framesIzquierda.append(spriteSheet.copy(i * w, 1 * h, w, h));
    for (int i = 0; i < 3; ++i)
        framesDerecha.append(spriteSheet.copy(i * w, 2 * h, w, h));
}

/**
 * @brief Movimiento y animación del enemigo en el nivel 1.
 * Incluye patrullaje, detección de Goku y disparo.
 */
void Enemigo::mover() {
    posX -= 3;
    setX(posX);
    if (gokuDetectado) {
        float distancia = std::abs(gokuDetectado->x() - x());
        if (distancia < 900 && gokuDetectado->x() < x()) {
            estado = Disparando;
            direccion = -1;
            if (!disparando) {
                disparando = true;
                timerDisparo->start(1000);
            }
        } else {
            estado = Patrullando;
            if (disparando) {
                disparando = false;
                timerDisparo->stop();
            }
        }
    }
    contador++;
    if (contador >= velocidadAnimacion) {
        QVector<QPixmap>& frames = (direccion == -1) ? framesIzquierda : framesDerecha;
        setPixmap(frames[frameActual].scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        frameActual = (frameActual + 1) % frames.size();
        contador = 0;
    }
    if (estado == Patrullando) {
        posX += direccion * velocidadMovimiento;
        setX(posX);
        if (posX <= 800 || posX >= 1300)
            direccion *= -1;
    }
}

/**
 * @brief Disparo del enemigo con proyectil rojo si mira hacia la izquierda.
 */
void Enemigo::disparar(QGraphicsScene* escena) {
    if (pausado || direccion != -1) return;
    QGraphicsEllipseItem* proyectilSoldado = new QGraphicsEllipseItem(0, 0, 15, 15);
    proyectilSoldado->setBrush(Qt::red);
    proyectilSoldado->setZValue(1);
    float posDisparoY = y() + boundingRect().height() / 2 - 7;
    float posDisparoX = x() - 75;
    proyectilSoldado->setPos(posDisparoX, posDisparoY);
    escena->addItem(proyectilSoldado);

    QTimer* t = new QTimer(this);
    connect(t, &QTimer::timeout, [=]() mutable {
        if (!proyectilSoldado || !proyectilSoldado->scene()) {
            t->stop(); t->deleteLater(); return;
        }
        proyectilSoldado->moveBy(-20, 0);
        if (proyectilSoldado->x() < -100) {
            if (scene()) scene()->removeItem(proyectilSoldado);
            delete proyectilSoldado;
            for (int i = 0; i < proyectilesActivos.size(); ++i) {
                if (proyectilesActivos[i].first == proyectilSoldado) {
                    QTimer* tmp = proyectilesActivos[i].second;
                    if (tmp) { tmp->stop(); tmp->deleteLater(); }
                    proyectilesActivos.removeAt(i);
                    break;
                }
            }
            t->stop(); t->deleteLater();
        }
    });
    t->start(30);
    proyectilesActivos.append(qMakePair(proyectilSoldado, t));
}

/** @brief Elimina todos los proyectiles activos del enemigo. */
void Enemigo::eliminarProyectiles() {
    for (const auto& par : proyectilesActivos) {
        auto proyectil = par.first;
        auto timer = par.second;
        if (scene()) scene()->removeItem(proyectil);
        delete proyectil;
        if (timer) { timer->stop(); timer->deleteLater(); }
    }
    proyectilesActivos.clear();
}

/** @brief Detiene los temporizadores de los proyectiles activos. */
void Enemigo::detenerProyectiles() {
    for (auto& par : proyectilesActivos) {
        if (par.second) par.second->stop();
    }
}

/** @brief Reanuda los temporizadores de los proyectiles activos. */
void Enemigo::reanudarProyectiles() {
    for (auto& par : proyectilesActivos) {
        if (par.second) par.second->start(30);
    }
}

/** @brief Cambia el estado de pausa y detiene o reanuda acciones del enemigo. */
void Enemigo::setPausado(bool pausa) {
    pausado = pausa;
    if (pausado) {
        if (timerDisparo && timerDisparo->isActive()) timerDisparo->stop();
        detenerProyectiles();
    } else {
        if (disparando && timerDisparo) timerDisparo->start(1000);
        reanudarProyectiles();
    }
}

/** @brief Carga los sprites del enemigo para el modo laberinto (nivel 2). */
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

/** @brief Movimiento automático del enemigo en nivel 2. */
void Enemigo::patrullar() {
    if (disparandoNivel2) return;
    int paso = 4;
    QPointF nuevaPos = pos();
    if (direccionActual == "arriba") nuevaPos.ry() -= paso;
    else if (direccionActual == "abajo") nuevaPos.ry() += paso;
    else if (direccionActual == "izquierda") nuevaPos.rx() -= paso;
    else if (direccionActual == "derecha") nuevaPos.rx() += paso;
    if (!colisionaConMuro(nuevaPos, scene())) {
        mover2(direccionActual);
    } else {
        QStringList nuevasDirecciones = { "arriba", "abajo", "izquierda", "derecha" };
        nuevasDirecciones.removeAll(direccionActual);
        direccionActual = nuevasDirecciones.at(QRandomGenerator::global()->bounded(nuevasDirecciones.size()));
    }
}

int Enemigo::getFrameActual() const { return frameActual; }
Personaje* Enemigo::getGoku() const { return goku; }
void Enemigo::setDisparandoNivel2(bool valor) { disparandoNivel2 = valor; }
bool Enemigo::isDisparandoNivel2() const { return disparandoNivel2; }

/** @brief Detiene todas las acciones del enemigo (nivel 2). */
void Enemigo::detener() {
    if (timerPatrulla && timerPatrulla->isActive()) timerPatrulla->stop();
    if (timerDisparo2 && timerDisparo2->isActive()) timerDisparo2->stop();
    detenerProyectiles();
}

/** @brief Reanuda todas las acciones del enemigo (nivel 2). */
void Enemigo::reanudar() {
    if (timerPatrulla && !timerPatrulla->isActive()) timerPatrulla->start(100);
    if (timerDisparo2 && !timerDisparo2->isActive()) timerDisparo2->start();
    reanudarProyectiles();
}
