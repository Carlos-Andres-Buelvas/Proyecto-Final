#include "goku.h"
#include "juego.h"
#include "enemigo.h"

#include <QPixmap>
#include <QBrush>
#include <QGraphicsScene>
#include <QDebug>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

/**
 * @brief Constructor de Goku
 * Inicializa el personaje, carga animaciones, configura temporizador de disparo,
 * y activa enfoque en nivel 2.
 */
Goku::Goku(float x, float y, float ancho, float alto)
    : Personaje(x, y, ancho, alto), frameActual(0), contador(0),
    velocidadAnimacion(10), disparando(false)
{
    cargarAnimaciones();
    setPixmap(framesCorrer[0].scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    disparoTimer = new QTimer(this);
    connect(disparoTimer, &QTimer::timeout, this, &Goku::animarDisparoFrame);

    // NIVEL 2:
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();
    tipo = "goku";
}

/**
 * @brief Carga los sprites para animaciones de correr, caer, disparar y cuerda.
 */
void Goku::cargarAnimaciones() {
    QPixmap spriteSheet(":/sprites/Pictures/goku_corre_se_cae.png");

    // Correr (8 frames)
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

    // Disparo (2 frames)
    QPixmap disparo(":/sprites/Pictures/goku_dispara.png");
    for (int i = 0; i < 2; ++i)
        framesDisparo.append(disparo.copy(i * 467, 0, 467, 267));

    // Cuerda
    spriteCuerda = QPixmap(":/sprites/Pictures/goku_agarrado.png");
}

/** @brief Salto de Goku al presionar W. */
void Goku::saltar() {
    if (!enSuelo) return;
    velocidadY = -22;
    enSuelo = false;
    teclaWSostenida = true;
}

/** @brief Animación continua de correr. */
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

/** @brief Animación de caída con 4 frames. */
void Goku::animarCaida() {
    if (framesCaer.isEmpty()) return;

    frameActual = (frameActual + 1) % framesCaer.size();
    setPixmap(framesCaer[frameActual].scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/** @brief Inicia animación de disparo activando temporizador. */
void Goku::animarDisparo() {
    if (disparando) return;
    disparando = true;
    frameDisparoActual = 0;
    disparoTimer->start(150);
}

/** @brief Cambia frame por frame la animación de disparo. */
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

/** @brief Muestra sprite cuando Goku está colgado en la cuerda. */
void Goku::animarCuerda() {
    setPixmap(spriteCuerda.scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/** @brief Acelera la caída al presionar la tecla S. */
void Goku::acelerarCaida() {
    velocidadY += 10;
}

bool Goku::estaDisparando() const {
    return disparando;
}

/** @brief Disparo de Goku (3 proyectiles amarillos). */
void Goku::disparar(QGraphicsScene* escena) {
    QMediaPlayer* sonidoDisparo = new QMediaPlayer;
    QAudioOutput* audioDisparo = new QAudioOutput;
    sonidoDisparo->setAudioOutput(audioDisparo);
    sonidoDisparo->setSource(QUrl("qrc:/sounds/Sounds/disparo.wav"));
    audioDisparo->setVolume(70);
    sonidoDisparo->play();

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
                timer->stop(); timer->deleteLater(); return;
            }

            proyectilGoku->moveBy(30, 0);

            if (listaEnemigos) {
                for (int j = 0; j < listaEnemigos->size(); ++j) {
                    Enemigo* enemigo = listaEnemigos->at(j);
                    if (proyectilGoku->collidesWithItem(enemigo)) {
                        enemigo->eliminarProyectiles();
                        escena->removeItem(enemigo);
                        if (Juego* juego = dynamic_cast<Juego*>(escena->views().first())) {
                            juego->aumentarContadorSoldados();
                        }
                        delete enemigo;
                        listaEnemigos->removeAt(j);
                        escena->removeItem(proyectilGoku);
                        delete proyectilGoku;
                        timer->stop(); timer->deleteLater();
                        return;
                    }
                }
            }

            if (proyectilGoku->x() > 1300) {
                escena->removeItem(proyectilGoku);
                delete proyectilGoku;
                timer->stop(); timer->deleteLater();
            }
        });
        timer->start(30);
    }
}

/** @brief Aplica física de caída, gravedad y animaciones según el estado. */
void Goku::mover() {
    if (cayendoLento && velocidadY > 0) velocidadY += gravedad * 0.01;
    else velocidadY += gravedad;

    posY += velocidadY;

    float limiteSuelo = 450;
    if (posY >= limiteSuelo) {
        posY = limiteSuelo;
        velocidadY = 0;
        enSuelo = true;
        if (!disparando) animarCorrer();
    } else {
        enSuelo = false;
    }

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
    if (velocidadY == 0) velocidadY = 1;
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

/** @brief Carga las animaciones para el modo cenital (nivel 2). */
void Goku::cargarAnimacionesNivel2() {
    QPixmap sheet(":/sprites/Pictures/goku_pixel.png");
    int originalW = 150;
    int originalH = 150;
    int tileW = 55;
    int tileH = 38;

    framesArriba.clear();
    framesAbajo.clear();
    framesIzquierda.clear();
    framesDerecha.clear();

    for (int i = 0; i < 4; ++i) {
        framesAbajo.append(sheet.copy(i * originalW, 0 * originalH, originalW, originalH).scaled(tileW, tileH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        framesIzquierda.append(sheet.copy(i * originalW, 1 * originalH, originalW, originalH).scaled(tileW, tileH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        framesDerecha.append(sheet.copy(i * originalW, 2 * originalH, originalW, originalH).scaled(tileW, tileH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        framesArriba.append(sheet.copy(i * originalW, 3 * originalH, originalW, originalH).scaled(tileW, tileH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    setPixmap(framesAbajo[0]);
}

/** @brief Captura teclas W, A, S, D y P para movimiento y disparo (nivel 2). */
void Goku::keyPressEvent(QKeyEvent* event) {
    if (!m_enabled) return;

    switch (event->key()) {
    case Qt::Key_W: mover2("arriba"); break;
    case Qt::Key_S: mover2("abajo"); break;
    case Qt::Key_A: mover2("izquierda"); break;
    case Qt::Key_D: mover2("derecha"); break;
    case Qt::Key_P:
        if (energia >= 100) {
            disparar2(scene());
            QMediaPlayer* sonidoDisparo = new QMediaPlayer;
            QAudioOutput* audioDisparo = new QAudioOutput;
            sonidoDisparo->setAudioOutput(audioDisparo);
            sonidoDisparo->setSource(QUrl("qrc:/sounds/Sounds/disparo.wav"));
            audioDisparo->setVolume(70);
            sonidoDisparo->play();
        }
        break;
    default: break;
    }
}
