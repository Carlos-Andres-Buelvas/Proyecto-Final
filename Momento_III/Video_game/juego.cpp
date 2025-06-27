#include "juego.h"
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QDebug>
#include <QGraphicsScene>

Juego::Juego(QWidget *parent) : QGraphicsView(parent) {
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 1024, 600);
    setScene(escena);
    setFixedSize(1024, 600);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::StrongFocus);

    // Fondo tipo scroll
    QPixmap fondoPixmap(":/fondos/Pictures/fondo_nivel1.png");
    fondoPixmap = fondoPixmap.scaledToHeight(600, Qt::FastTransformation);

    for (int i = 0; i < 2; ++i) {
        auto fondo = new QGraphicsPixmapItem(fondoPixmap);
        fondo->setPos(i * (fondoPixmap.width() - 5), 0);
        fondo->setZValue(-1);
        escena->addItem(fondo);
        fondosScroll.append(fondo);
    }

    // Crear Goku
    goku = new Goku(0, 375, 275, 275);
    escena->addItem(goku);

    // Crear primer enemigo (opcional)
    Enemigo* enemigo = new Enemigo(1024, 375, 100, 100, goku);
    escena->addItem(enemigo);
    enemigos.append(enemigo);

    // Timer principal del juego
    timerJuego = new QTimer(this);
    connect(timerJuego, &QTimer::timeout, this, &Juego::actualizar);

    // Timer para generar enemigos
    timerEnemigos = new QTimer(this);
    connect(timerEnemigos, &QTimer::timeout, this, &Juego::generarEnemigo);

    // Barra de energía
    fondoBarra = new QGraphicsRectItem(0, 0, 104, 24);
    fondoBarra->setBrush(Qt::black);
    fondoBarra->setPos(10, 10);
    escena->addItem(fondoBarra);

    barraEnergia = new QGraphicsRectItem(fondoBarra);
    barraEnergia->setRect(2, 2, 0, 20);
    barraEnergia->setBrush(Qt::green);

    // Timer de cápsulas
    timerCapsulas = new QTimer(this);
    connect(timerCapsulas, &QTimer::timeout, this, [=]() {
        QPixmap sprite(":/sprites/Pictures/capsula.png");
        auto capsula = new QGraphicsPixmapItem(sprite.scaled(40, 40));

        int posY = 375;
        int variacion = QRandomGenerator::global()->bounded(-20, 20);
        capsula->setPos(1024, posY + variacion); // aparece fuera de la pantalla
        capsula->setZValue(2);
        escena->addItem(capsula);
        capsulas.append(capsula);
    });
    timerCapsulas->start(2000); // cada 2 segundos
}

void Juego::iniciar() {
    timerJuego->start(16);         // ~60 FPS
    timerEnemigos->start(5000);    // enemigos cada 1.5 s
}

void Juego::actualizar() {
    // Movimiento automático
    goku->setX(goku->x() + velocidadScroll);
    goku->mover();            // Aplica gravedad y salto
    goku->animarCorrer();     // Anima correr

    if (goku->x() > 500) {
        goku->setX(500); // Limita la posición de Goku para no salirse
    }

    // Scroll del fondo
    for (auto fondo : fondosScroll) {
        fondo->setX(fondo->x() - velocidadScroll);

        if (fondo->x() + fondo->pixmap().width() < 0) {
            float maxX = 0;
            for (auto f : fondosScroll)
                if (f != fondo)
                    maxX = qMax(maxX, f->x());

            fondo->setX(maxX + fondo->pixmap().width() - 5);
        }
    }

    // Mover cápsulas
    for (int i = 0; i < capsulas.size(); ++i) {
        auto c = capsulas[i];
        c->setX(c->x() - velocidadScroll);

        if (goku->collidesWithItem(c)) {
            escena->removeItem(c);
            delete c;
            capsulas.removeAt(i);
            goku->aumentarEnergia(20);
            actualizarBarraEnergia();
            --i;
        }
    }

    // Mover proyectiles
    for (int i = 0; i < proyectiles.size(); ++i) {
        auto p = proyectiles[i];
        p->setX(p->x() + 10);

        if (p->x() > 1300) {
            escena->removeItem(p);
            delete p;
            proyectiles.removeAt(i);
            --i;
        }
    }

    // Aumentar velocidad gradualmente
    static int contador = 0;
    contador++;

    if (contador % 300 == 0 && velocidadScroll < 10) {
        velocidadScroll += 0.2;
    }

    // Mover enemigos y eliminarlos si se salen
    for (int i = 0; i < enemigos.size(); ++i) {
        Enemigo* enemigo = enemigos[i];

        enemigo->mover();

        if (enemigo->x() + enemigo->boundingRect().width() < 0) {
            escena->removeItem(enemigo);
            delete enemigo;
            enemigos.removeAt(i);
            --i;
        }
    }

    for (int i = 0; i < proyectiles.size(); ++i) {
        auto p = proyectiles[i];
        p->setX(p->x() + 10);

        for (int j = 0; j < enemigos.size(); ++j) {
            Enemigo* enemigo = enemigos[j];
            if (p->collidesWithItem(enemigo)) {
                escena->removeItem(p);
                escena->removeItem(enemigo);
                delete p;
                delete enemigo;
                proyectiles.removeAt(i);
                enemigos.removeAt(j);
                --i;
                break;
            }
        }

        if (p->x() > 1300) {
            escena->removeItem(p);
            delete p;
            proyectiles.removeAt(i);
            --i;
        }
    }

    // Colisión Goku vs Enemigo
    for (int i = 0; i < enemigos.size(); ++i) {
        if (goku->collidesWithItem(enemigos[i])) {
            // Puedes mostrar alguna animación, sonido o mensaje
            qDebug() << "Goku ha colisionado con un enemigo.";
            // Aquí puedes reducir vida, reiniciar juego, o lo que necesites
            return;
        }
    }
}

void Juego::generarEnemigo() {
    int posY = 375; // siempre en el mismo nivel del suelo que Goku

    Enemigo* enemigo = new Enemigo(1024, posY, 100, 100, goku); // misma altura
    escena->addItem(enemigo);
    enemigos.append(enemigo);
}

void Juego::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) {
        goku->saltar();
        goku->cayendoLento = true;
    }
    if (event->key() == Qt::Key_S) {
        goku->acelerarCaida();
    }
    if (event->key() == Qt::Key_Space) {
        if (goku->puedeDisparar()) {
            goku->animarDisparo();

            for (int i = 0; i < 3; ++i) {
                auto p = goku->crearProyectil();
                p->setY(p->y() - i * 10); // Pequeña separación vertical opcional
                escena->addItem(p);
                proyectiles.append(p);
            }

            goku->reiniciarEnergia();
            actualizarBarraEnergia();
        }
    }
}

void Juego::actualizarBarraEnergia() {
    float porcentaje = static_cast<float>(goku->obtenerEnergia()) / 100.0f;
    barraEnergia->setRect(2, 2, porcentaje * 100, 20);
}

void Juego::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) {
        goku->cayendoLento = false;
    }
}
