#include "juego.h"
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QDebug>

Juego::Juego(QWidget *parent) : QGraphicsView(parent) {
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 1024, 600);
    setScene(escena);
    setFixedSize(1024, 600);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::StrongFocus);

    crearScrollFondo();

    goku = new Goku(0, 375, 275, 275);
    escena->addItem(goku);

    timerJuego = new QTimer(this);
    connect(timerJuego, &QTimer::timeout, this, &Juego::actualizar);

    timerEnemigos = new QTimer(this);
    connect(timerEnemigos, &QTimer::timeout, this, &Juego::generarEnemigo);

    // Movimiento automático + gravedad continua
    timerInput = new QTimer(this);
    connect(timerInput, &QTimer::timeout, this, [=]() {
        goku->setX(goku->x() + 3); // Goku avanza solo
        goku->mover();             // Aplica gravedad y salto
        goku->animarCorrer();      // Anima mientras corre
    });
    timerInput->start(16); // ~60 FPS

}

void Juego::iniciar() {
    timerJuego->start(16); // ~60 fps
    timerEnemigos->start(2000); // cada 2 segundos
}

void Juego::crearScrollFondo() {
    QPixmap fondoImg(":/fondos/Pictures/fondo_nivel1.png");
    fondoImg = fondoImg.scaled(1024, 600);

    for (int i = 0; i < 2; ++i) {
        QGraphicsPixmapItem* fondo = new QGraphicsPixmapItem(fondoImg);
        fondo->setZValue(-1); // al fondo
        fondo->setPos(i * 800, 0);
        escena->addItem(fondo);
        fondos.append(fondo);
    }
}

void Juego::moverFondo() {
    for (auto fondo : fondos) {
        fondo->moveBy(-2, 0);
        if (fondo->x() + 800 < 0) {
            fondo->setX(800);
        }
    }
}

void Juego::actualizar() {
    goku->mover();
    moverFondo();

    // Aquí se puede agregar detección de colisiones
}

void Juego::generarEnemigo() {
    Enemigo* enemigo = new Enemigo(850, 400, 80, 80);
    escena->addItem(enemigo);

    QTimer* enemigoTimer = new QTimer(this);
    connect(enemigoTimer, &QTimer::timeout, [enemigo]() {
        enemigo->mover();
        if (enemigo->x() < -100) {
            enemigo->scene()->removeItem(enemigo);
            delete enemigo;
        }
    });
    enemigoTimer->start(30);
}

void Juego::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) {
        goku->saltar(); // Salta con impulso hacia arriba
    }
    if (event->key() == Qt::Key_S) {
        goku->acelerarCaida(); // Baja más rápido
    }
}
