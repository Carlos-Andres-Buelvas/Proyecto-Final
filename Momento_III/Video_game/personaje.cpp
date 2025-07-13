#include "personaje.h"
#include "goku.h"
#include "enemigo.h"
#include "juego2.h"
#include <QGraphicsScene>

/**
 * @brief Constructor base para cualquier personaje.
 * @param x Posición inicial en X.
 * @param y Posición inicial en Y.
 * @param ancho Ancho del sprite.
 * @param alto Alto del sprite.
 */
Personaje::Personaje(float x, float y, float ancho, float alto)
    : posX(x), posY(y), velocidadY(0), gravedad(1), ancho(ancho), alto(alto)
{
    setPos(posX, posY);  // Establece la posición inicial en la escena
}

// NIVEL 2:

/**
 * @brief Mueve al personaje en una dirección dada y actualiza el sprite.
 *
 * @param direccion Dirección a mover: "arriba", "abajo", "izquierda", "derecha".
 */
void Personaje::mover2(const QString& direccion) {
    int paso = 4;
    QPointF nuevaPos = pos();  // posición actual

    if (direccion == "arriba") nuevaPos.ry() -= paso;
    else if (direccion == "abajo") nuevaPos.ry() += paso;
    else if (direccion == "izquierda") nuevaPos.rx() -= paso;
    else if (direccion == "derecha") nuevaPos.rx() += paso;

    // Guardar dirección actual solo si es Goku
    if (tipo == "goku"){
        Goku* g = dynamic_cast<Goku*>(this);
        if (g) g->direccionActual = direccion;
    }

    // Verificar colisión con muros
    if (!colisionaConMuro(nuevaPos, scene())) {
        setPos(nuevaPos);  // Solo si no choca
        posX = nuevaPos.x();
        posY = nuevaPos.y();
    }

    // Animación de movimiento
    if (direccion == "arriba")
        setPixmap(framesArriba[frameActual++ % framesArriba.size()]);
    else if (direccion == "abajo")
        setPixmap(framesAbajo[frameActual++ % framesAbajo.size()]);
    else if (direccion == "izquierda")
        setPixmap(framesIzquierda[frameActual++ % framesIzquierda.size()]);
    else if (direccion == "derecha")
        setPixmap(framesDerecha[frameActual++ % framesDerecha.size()]);
}

/**
 * @brief Lanza un proyectil en la dirección del personaje (Goku o Enemigo).
 *
 * Goku dispara proyectiles amarillos en la dirección actual,
 * mientras que los enemigos disparan hacia Goku con proyectiles rojos.
 *
 * @param escena Escena donde se agregan los proyectiles.
 */
void Personaje::disparar2(QGraphicsScene* escena) {
    if (tipo == "goku") {
        Goku* goku = dynamic_cast<Goku*>(this);
        if (!goku || !escena) return;

        int dx = 0, dy = 0;
        QString dir = goku->direccionActual;

        if (dir == "arriba") dy = -15;
        else if (dir == "abajo") dy = 15;
        else if (dir == "izquierda") dx = -15;
        else dx = 15;  // derecha por defecto

        QGraphicsEllipseItem* bola = new QGraphicsEllipseItem(0, 0, 10, 10);
        bola->setBrush(Qt::yellow);
        bola->setZValue(5);
        bola->setPos(x() + boundingRect().width() / 2, y() + boundingRect().height() / 2);
        escena->addItem(bola);

        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, [=]() mutable {
            bola->moveBy(dx, dy);

            QVector<QGraphicsItem*> colisiones = escena->items(bola->sceneBoundingRect());
            for (auto item : colisiones) {
                if (item != bola && (item->data(0) == "muro" || item->data(0) == "enemigo")) {
                    escena->removeItem(bola);
                    delete bola;
                    timer->stop();
                    delete timer;

                    if (item->data(0) == "enemigo") {
                        Enemigo* enemigo = dynamic_cast<Enemigo*>(item);
                        if (enemigo) {
                            Juego2* juego2 = qobject_cast<Juego2*>(escena->views().first());
                            if (juego2) {
                                juego2->eliminarEnemigo(enemigo);  // limpio, seguro y público
                            }
                        }
                    }
                }
            }

            if (bola->x() < 0 || bola->x() > escena->width() ||
                bola->y() < 0 || bola->y() > escena->height()) {
                escena->removeItem(bola);
                delete bola;
                timer->stop();
                delete timer;
            }
        });

        timer->start(20);
        goku->reiniciarEnergia();

        QTimer::singleShot(1, goku, [=](){
            emit actualizarBarraEnergiaSignal();
        });

    } else if (tipo == "enemigo") {
        if (proyectilesActivos.size() >= 30) return;

        Enemigo* enemigo = dynamic_cast<Enemigo*>(this);
        if (!enemigo || !escena || !enemigo->getGoku()) return;

        enemigo->setDisparandoNivel2(true);

        QPointF objetivo = enemigo->getGoku()->pos();
        QPointF origen = pos();
        QPointF direccion = objetivo - origen;
        qreal dx = 0, dy = 0;

        // Dirección del disparo y animación
        if (std::abs(direccion.x()) > std::abs(direccion.y())) {
            dx = (direccion.x() > 0) ? 10 : -10;
            enemigo->setPixmap(
                dx > 0
                    ? enemigo->framesDerecha[enemigo->getFrameActual() % enemigo->framesDerecha.size()]
                    : enemigo->framesIzquierda[enemigo->getFrameActual() % enemigo->framesIzquierda.size()]
                );
        } else {
            dy = (direccion.y() > 0) ? 10 : -10;
            enemigo->setPixmap(
                dy > 0
                    ? enemigo->framesAbajo[enemigo->getFrameActual() % enemigo->framesAbajo.size()]
                    : enemigo->framesArriba[enemigo->getFrameActual() % enemigo->framesArriba.size()]
                );
        }

        QTimer* timer = new QTimer();
        QGraphicsEllipseItem* bola = new QGraphicsEllipseItem(0, 0, 10, 10);
        bola->setBrush(Qt::red);
        bola->setZValue(5);
        bola->setPos(x() + boundingRect().width() / 2, y() + boundingRect().height() / 2);
        escena->addItem(bola);
        proyectilesActivos.append(qMakePair(bola, timer));

        connect(timer, &QTimer::timeout, [=]() mutable {
            bola->moveBy(dx, dy);

            QVector<QGraphicsItem*> colisiones = escena->items(bola->sceneBoundingRect());
            for (auto item : colisiones) {
                if (item != bola) {
                    if (item->data(0) == "muro") {
                        escena->removeItem(bola);
                        delete bola;
                        timer->stop();
                        delete timer;
                        return;
                    }

                    Goku* goku = dynamic_cast<Goku*>(item);
                    if (goku) {
                        escena->removeItem(bola);
                        delete bola;
                        timer->stop();
                        delete timer;

                        qDebug() << "Goku fue alcanzado por un disparo. Fin del juego.";
                        QObject* parentView = escena->views().isEmpty() ? nullptr : escena->views().first();
                        Juego2* juego2 = qobject_cast<Juego2*>(parentView);
                        if (juego2) {
                            juego2->mostrarGameOver();
                        }
                        return;
                    }
                }
            }

            if (bola->x() < 0 || bola->x() > escena->width() ||
                bola->y() < 0 || bola->y() > escena->height()) {
                escena->removeItem(bola);
                delete bola;
                timer->stop();
                delete timer;
            }
        });

        timer->start(25);

        QTimer::singleShot(1000, enemigo, [enemigo]() {
            enemigo->setDisparandoNivel2(false);
        });
    }
}

/**
 * @brief Verifica si una nueva posición colisiona con un muro o puerta.
 * @param nuevaPos Nueva posición tentativa.
 * @param escena Escena en la que se verifica la colisión.
 * @return true si hay colisión con un muro o puerta; false si puede moverse.
 */
bool Personaje::colisionaConMuro(const QPointF& nuevaPos, QGraphicsScene* escena) {
    int margen = 6; // Ajuste de sensibilidad

    QRectF rectNuevo = QRectF(
        nuevaPos.x() + margen,
        nuevaPos.y() + margen,
        boundingRect().width() - 2 * margen,
        boundingRect().height() - 2 * margen
        );

    QVector<QGraphicsItem*> items = escena->items(rectNuevo);

    for (QGraphicsItem* item : items) {
        if (item != this && (item->data(0) == "muro" || item->data(0) == "puerta")) {
            return true;
        }
    }

    return false;
}
