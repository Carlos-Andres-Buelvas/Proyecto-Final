#include "goku.h"
#include "juego.h"
#include "enemigo.h"
#include <QPixmap>
#include <QBrush>
#include <QGraphicsScene>
#include <QDebug>

Goku::Goku(float x, float y, float ancho, float alto)
    : Personaje(x, y, ancho, alto), frameActual(0), contador(0),
    velocidadAnimacion(10), disparando(false)
{
    cargarAnimaciones();
    setPixmap(framesCorrer[0].scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    disparoTimer = new QTimer(this);
    connect(disparoTimer, &QTimer::timeout, this, &Goku::animarDisparoFrame);

}

void Goku::cargarAnimaciones() {
    QPixmap spriteSheet(":/sprites/Pictures/goku_corre_se_cae.png");

    // Correr (8 frames, 3 columnas)
    for (int i = 0; i < 8; ++i) {
        int x = (i % 3) * 1;
        int y = (i / 3) * 1;
        framesCorrer.append(spriteSheet.copy(x * 498, y * 285, 498, 285));
    }

    // Caída (4 frames)
    for (int i = 8; i < 12; ++i) {
        int x = (i % 3) * 498;
        int y = (i / 3) * 285;
        framesCaer.append(spriteSheet.copy(x, y, 498, 285));
    }

    // Disparo
    QPixmap disparo(":/sprites/Pictures/goku_dispara.png");
    for (int i = 0; i < 2; ++i)
        framesDisparo.append(disparo.copy(i * 467, 0, 467, 267));

    // Cuerda
    spriteCuerda = QPixmap(":/sprites/Pictures/goku_agarrado.png");
}

void Goku::saltar() {
    if (!enSuelo) return;

    velocidadY = -22;
    enSuelo = false;
    teclaWSostenida = true;
}

void Goku::animarCorrer() {
    contador++;
    if (contador >= velocidadAnimacion) {
        QPixmap frame = framesCorrer[frameActual];
        QPixmap escalado = frame.scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setOffset((ancho - escalado.width()) / 2, (alto - escalado.height()) / 2);
        setPixmap(escalado);

        frameActual = (frameActual + 1) % framesCorrer.size();
        contador = 0;
    }
}

void Goku::animarCaida() {
    if (framesCaer.isEmpty()) return;

    frameActual = (frameActual + 1) % framesCaer.size();
    setPixmap(framesCaer[frameActual].scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Goku::animarDisparo() {
    if (disparando) return; // Evita que se interrumpa si ya está disparando

    disparando = true;
    frameDisparoActual = 0;
    disparoTimer->start(150); // Velocidad entre frames
}

void Goku::animarDisparoFrame() {
    const int repeticiones = 6;

    if (frameDisparoActual < repeticiones) {
        int index = frameDisparoActual % framesDisparo.size();
        QPixmap frame = framesDisparo[index];
        QPixmap escalado = frame.scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setOffset((ancho - escalado.width()) / 2, (alto - escalado.height()) / 2);
        setPixmap(escalado);

        frameDisparoActual++;
    } else {
        disparoTimer->stop();
        disparando = false;
        animarCorrer();
    }
}

void Goku::animarCuerda() {
    setPixmap(spriteCuerda.scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Goku::acelerarCaida() {
    velocidadY += 10;
}

bool Goku::estaDisparando() const {
    return disparando;
}

void Goku::disparar(QGraphicsScene* escena) {
    for (int i = 0; i < 3; ++i) {
        QGraphicsEllipseItem* proyectilGoku = new QGraphicsEllipseItem(0, 0, 30, 30);
        proyectilGoku->setBrush(Qt::yellow);
        proyectilGoku->setZValue(1);

        float posX = x() + ancho;
        float posY = y() + alto / 2 - 15 - (i * 10);
        proyectilGoku->setPos(posX, posY);

        escena->addItem(proyectilGoku);

        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, [=]() mutable {
            if (!proyectilGoku || !proyectilGoku->scene()) {
                timer->stop();
                timer->deleteLater();
                return;
            }

            proyectilGoku->moveBy(30, 0);

            // 🎯 DETECCIÓN DE COLISIÓN CON ENEMIGOS SIN CAMBIAR FIRMA
            if (listaEnemigos) {
                for (int j = 0; j < listaEnemigos->size(); ++j) {
                    Enemigo* enemigo = listaEnemigos->at(j);
                    if (proyectilGoku->collidesWithItem(enemigo)) {
                        enemigo->eliminarProyectiles();
                        escena->removeItem(enemigo);
                        delete enemigo;
                        listaEnemigos->removeAt(j);

                        escena->removeItem(proyectilGoku);
                        delete proyectilGoku;

                        timer->stop();
                        timer->deleteLater();
                        return;
                    }
                }
            }

            if (proyectilGoku->x() > 1300) {
                escena->removeItem(proyectilGoku);
                delete proyectilGoku;
                timer->stop();
                timer->deleteLater();
            }
        });

        timer->start(30);
    }
}

void Goku::mover() {
    // Aplicar gravedad con lógica de caída lenta
    if (cayendoLento && velocidadY > 0) {
        velocidadY += gravedad * 0.01; // Caída más lenta
    } else {
        velocidadY += gravedad;     // Caída normal
    }

    posY += velocidadY;

    float limiteSuelo = 375;
    if (posY >= limiteSuelo) {
        posY = limiteSuelo;
        velocidadY = 0;
        enSuelo = true;

        if (!disparando)
            animarCorrer();  // Goku corre cuando está en el suelo
    } else {
        enSuelo = false;
    }

    // Ya no se modifica posX aquí, se hace en Juego::actualizar()
    setPos(posX, posY);
}

void Goku::setListaEnemigos(QVector<Enemigo*>* lista) {
    listaEnemigos = lista;
}

bool Goku::estaBajando() const {
    return velocidadY > 0;
}

bool Goku::estaEnSuelo() const {
    return enSuelo;
}

void Goku::detenerCaida() {
    velocidadY = 0;
    enSuelo = true;
}

void Goku::activarCaida() {
    enSuelo = false;
    // Si Goku ya tiene velocidad negativa (subiendo), no se toca
    // Solo lo aseguramos cayendo si está flotando sin estar en suelo
    if (velocidadY == 0)
        velocidadY = 1; // mínima caída para reiniciar el efecto de gravedad
}

void Goku::forzarCaida() {
    forzarCaidaManual = true;
}

bool Goku::estaForzandoCaida() const {
    return forzarCaidaManual;
}

void Goku::cancelarCaidaForzada() {
    forzarCaidaManual = false;
}
