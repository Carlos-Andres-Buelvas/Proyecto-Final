#include "juego.h"
#include "enemigo.h"
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
    goku->setZValue(2);
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

    timerPlataformas = new QTimer(this);
    connect(timerPlataformas, &QTimer::timeout, this, &Juego::generarPlataforma);
    timerPlataformas->start(3500);  // cada 3.5 segundos (puedes ajustar)
}

void Juego::iniciar() {
    timerJuego->start(16);         // ~60 FPS
    timerEnemigos->start(7000);    // enemigos cada 1.5 s
}

void Juego::actualizar() {
    // Movimiento automático
    goku->setX(goku->x() + velocidadScroll);
    goku->mover();            // Aplica gravedad y salto

    bool sobrePlataforma = false;

    for (auto plataforma : plataformas) {
        if (goku->collidesWithItem(plataforma)) {
            // Verificamos que la colisión sea desde arriba (evitar que se "suba" desde abajo o de lado)
            qreal gokuBaseY = goku->y() + goku->boundingRect().height();
            qreal plataformaY = plataforma->y();

            if (gokuBaseY <= plataformaY + 15 && goku->estaBajando() && !goku->estaForzandoCaida()) {
                // Aterrizó en la plataforma
                goku->setY(plataformaY - goku->boundingRect().height() - 30);
                goku->detenerCaida();
                sobrePlataforma = true;
                break;
            }
        }
    }

    // Si no está ni en plataforma ni en el suelo, cae
    if (!sobrePlataforma && !goku->estaEnSuelo()) {
        goku->activarCaida();
        goku->cancelarCaidaForzada();  // ✅ una vez que cae, ya no está forzando
    }

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
            enemigo->eliminarProyectiles();
            escena->removeItem(enemigo);
            delete enemigo;
            enemigos.removeAt(i);
            --i;
        }
    }

    // Colisión Goku vs Enemigo
    for (int i = 0; i < enemigos.size(); ++i) {
        if (goku->collidesWithItem(enemigos[i])) {
            goku->animarCaida();
            // Puedes mostrar alguna animación, sonido o mensaje
            qDebug() << "Goku ha colisionado con un enemigo.";
            // Aquí puedes reducir vida, reiniciar juego, o lo que necesites
            return;
        }
    }

    // Colisión Goku vs proyectiles de soldados
    for (int i = 0; i < enemigos.size(); ++i) {
        Enemigo* enemigo = enemigos[i];

        for (int j = 0; j < enemigo->proyectilesActivos.size(); ++j) {
            QGraphicsEllipseItem* proyectil = enemigo->proyectilesActivos[j].first;

            if (proyectil && goku->collidesWithItem(proyectil)) {
                goku->animarCaida();
                qDebug() << "Goku ha sido golpeado por un disparo enemigo.";
                return;
            }
        }
    }

    for (int i = 0; i < plataformas.size(); ++i) {
        auto p = plataformas[i];
        p->setX(p->x() - velocidadScroll);

        if (p->x() + p->pixmap().width() < 0) {
            escena->removeItem(p);
            delete p;
            plataformas.removeAt(i);
            --i;
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
        goku->forzarCaida();
    }
    if (event->key() == Qt::Key_Space) {
        if (goku->puedeDisparar()) {
            goku->animarDisparo();
            goku->setListaEnemigos(&enemigos);
            goku->disparar(escena);  // ✅ llama al método heredado correctamente

            goku->reiniciarEnergia();
            actualizarBarraEnergia();
        }
    }
}

void Juego::actualizarBarraEnergia() {
    float porcentaje = static_cast<float>(goku->obtenerEnergia()) / 100.0f;
    barraEnergia->setRect(2, 2, porcentaje * 100, 20);
}

void Juego::generarPlataforma() {
    QVector<QString> rutas = {
        ":/sprites/Pictures/plataforma_1.png",
        ":/sprites/Pictures/plataforma_2.png",
        ":/sprites/Pictures/plataforma_3.png"
    };

    int cantidad = QRandomGenerator::global()->bounded(1, 3); // 1 o 2 plataformas
    int baseX = 1024;

    for (int i = 0; i < cantidad; ++i) {
        QString ruta = rutas[QRandomGenerator::global()->bounded(0, rutas.size())];
        QPixmap sprite(ruta);
        sprite = sprite.scaled(350, 35); // más ancha

        QGraphicsPixmapItem* plataforma = new QGraphicsPixmapItem(sprite);
        plataforma->setZValue(1);

        int y;
        if (cantidad == 2) {
            // Si hay 2, la primera más baja, la segunda más alta
            y = (i == 0) ? 425 : 250;
        } else {
            // Si solo hay una, se escoge aleatoriamente
            y = (QRandomGenerator::global()->bounded(0, 2) == 0) ? 425 : 250;
        }

        plataforma->setPos(baseX + i * 400, y);  // separadas horizontalmente
        escena->addItem(plataforma);
        plataformas.append(plataforma);
    }
}

void Juego::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) {
        goku->cayendoLento = false;
    }
}
