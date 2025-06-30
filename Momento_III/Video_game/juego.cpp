#include "juego.h"
#include "enemigo.h"
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QDebug>
#include <QGraphicsScene>
#include <QPixmap>

Juego::Juego(QWidget *parent) : QGraphicsView(parent) {
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 1280, 680);
    setScene(escena);
    setFixedSize(1280, 680);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::StrongFocus);

    // Crear cuerda
    origenCuerda = QPointF(1000, 0);  // esquina superior derecha
    cuerdaLine = new QGraphicsLineItem();
    cuerdaLine->setPen(QPen(Qt::darkYellow, 4));
    escena->addItem(cuerdaLine);

    // Sprite Goku colgado (oculto al inicio)
    QPixmap gokuSprite(":/sprites/Pictures/goku_agarrado.png");
    gokuColgado = new QGraphicsPixmapItem(gokuSprite.scaled(90, 200));
    gokuColgado->setVisible(false);
    gokuColgado->setZValue(3);
    escena->addItem(gokuColgado);

    // Timer para actualizar el movimiento de la cuerda
    timerCuerda = new QTimer(this);
    connect(timerCuerda, &QTimer::timeout, this, &Juego::actualizarCuerda);
    timerCuerda->start(16);  // 60 fps

    // Cargar y guardar plataformas
    imagenesPlataformas = {
        QPixmap(":/sprites/Pictures/plataforma_1.png").scaled(550, 35),
        QPixmap(":/sprites/Pictures/plataforma_2.png").scaled(550, 35),
        QPixmap(":/sprites/Pictures/plataforma_3.png").scaled(550, 35)
    };

    // Cargar troncos
    imagenesTroncos = {
        QPixmap(":/sprites/Pictures/tronco_1.png").scaledToWidth(75),
        QPixmap(":/sprites/Pictures/tronco_2.png").scaledToWidth(75),
        QPixmap(":/sprites/Pictures/tronco_3.png").scaledToWidth(75)
    };

    // Cargar rocas
    imagenesRocas = {
        QPixmap(":/sprites/Pictures/roca_1.png").scaledToWidth(75),
        QPixmap(":/sprites/Pictures/roca_2.png").scaledToWidth(75),
        QPixmap(":/sprites/Pictures/roca_3.png").scaledToWidth(75)
    };

    imagenesObstaculos += imagenesTroncos;
    imagenesObstaculos += imagenesRocas;

    // Fondo tipo scroll
    QPixmap fondoPixmap(":/fondos/Pictures/fondo_nivel1.png");
    fondoPixmap = fondoPixmap.scaledToHeight(680, Qt::FastTransformation);

    for (int i = 0; i < 2; ++i) {
        auto fondo = new QGraphicsPixmapItem(fondoPixmap);
        fondo->setPos(i * (fondoPixmap.width() - 5), 0);
        fondo->setZValue(-1);
        escena->addItem(fondo);
        fondosScroll.append(fondo);
    }

    // Crear Goku
    goku = new Goku(0, 450, 275, 275);
    goku->setZValue(2);
    escena->addItem(goku);

    // Crear primer enemigo (opcional)
    Enemigo* enemigo = new Enemigo(1280, 650, 100, 100, goku);
    escena->addItem(enemigo);
    enemigos.append(enemigo);
    generarPlataforma();

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
        int cantidad = QRandomGenerator::global()->bounded(1, 4);  // 1 a 3 cápsulas

        QVector<QGraphicsPixmapItem*> plataformasVisibles;
        for (auto p : plataformas) {
            if (p->x() > 100 && p->x() + p->pixmap().width() < 1024) {
                plataformasVisibles.append(p);
            }
        }

        for (int i = 0; i < cantidad; ++i) {
            QPixmap sprite(":/sprites/Pictures/capsula.png");
            auto capsula = new QGraphicsPixmapItem(sprite.scaled(40, 40));
            capsula->setZValue(2);

            int posX = 1024 + i * 45;
            int posY = 475;  // por defecto, en el suelo

            // Si hay plataformas disponibles, poner cápsula encima de alguna de ellas
            bool enPlataforma = QRandomGenerator::global()->bounded(0, 100) < 50;
            if (enPlataforma && !plataformasVisibles.isEmpty()) {
                auto plataforma = plataformasVisibles.at(QRandomGenerator::global()->bounded(plataformasVisibles.size()));
                posY = plataforma->y() - 40; // encima
                posX = plataforma->x() + QRandomGenerator::global()->bounded(0, plataforma->pixmap().width() - 40);
            }

            capsula->setPos(posX, posY);
            escena->addItem(capsula);
            capsulas.append(capsula);
        }
    });
    timerCapsulas->start(3000); // cada 2 segundos

    timerPlataformas = new QTimer(this);
    connect(timerPlataformas, &QTimer::timeout, this, &Juego::generarPlataforma);
    timerPlataformas->start(5000);  // cada 3.5 segundos (puedes ajustar)

    // Timer para generar obstáculos cada 3 segundos (ajústalo a tu gusto)
    timerObstaculos = new QTimer(this);
    connect(timerObstaculos, &QTimer::timeout, this, &Juego::generarObstaculo);
    timerObstaculos->start(6000);
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

    // Verificar si Goku toca la cuerda
    if (!gokuEnCuerda && goku->y() < 250) {  // altura de salto
        QLineF cuerda = cuerdaLine->line();
        QPointF extremo = cuerda.p2();

        QRectF gokuRect = goku->boundingRect().translated(goku->pos());
        QRectF puntoCuerda(extremo.x() - 15, extremo.y() - 15, 30, 30);  // pequeña área

        if (gokuRect.intersects(puntoCuerda)) {
            gokuEnCuerda = true;

            // Oculta Goku normal y muestra colgado
            goku->setVisible(false);
            gokuColgado->setVisible(true);

            // Goku empieza desde el extremo
            gokuColgado->setPos(extremo.x() - gokuColgado->pixmap().width() / 2,
                                extremo.y());
        }
    }

    goku->animarCorrer();     // Anima correr
    if (goku->x() > 700) {
        goku->setX(700); // Limita la posición de Goku para no salirse
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

    // Scroll para la cuerda
    origenCuerda.setX(origenCuerda.x() - velocidadScroll);

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
                //goku->animarCaida();
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

    // Mover obstáculos y eliminarlos si salen de pantalla
    for (int i = 0; i < obstaculos.size(); ++i) {
        auto o = obstaculos[i];
        o->setX(o->x() - velocidadScroll);

        if (o->x() + o->pixmap().width() < 0) {
            escena->removeItem(o);
            delete o;
            obstaculos.removeAt(i);
            --i;
        }
    }

    // Colisión Goku vs obstáculo con verificación de altura correcta
    for (auto o : obstaculos) {
        if (goku->collidesWithItem(o)) {
            qreal obstY = o->y();
            qreal gokuY = goku->y();
            qDebug() << obstY << gokuY;
            qDebug() << std::abs(obstY - gokuY);

            // ⚠️ Tolerancia para colisión en el suelo
            if (std::abs(obstY - gokuY) < 180) {
                goku->animarCaida();
                qDebug() << "✅ Goku ha chocado con obstáculo en el suelo.";
                return;
            }

            // ✔️ Caso 2: obstáculo sobre plataforma — verificar plataforma común
            for (auto p : plataformas) {
                qreal platY = p->y();
                qreal gokuBase = goku->y() + goku->boundingRect().height();
                qreal obstBase = o->y() + o->boundingRect().height();

                bool obstSobrePlataforma = std::abs(obstBase - platY) <= 5;
                bool gokuSobrePlataforma = std::abs(gokuBase - platY) <= 5;

                qDebug() << std::abs(obstBase - platY);
                 qDebug() << std::abs(gokuBase - platY);

                if (obstSobrePlataforma && gokuSobrePlataforma &&
                    p->collidesWithItem(o) && p->collidesWithItem(goku)) {

                    goku->animarCaida();
                    qDebug() << "✅ Goku ha chocado con obstáculo sobre plataforma.";
                    return;
                }
            }
        }
    }
}

void Juego::generarEnemigo() {
    int cantidad = QRandomGenerator::global()->bounded(1, 3); // 1 o 2 soldados

    for (int i = 0; i < cantidad; ++i) {
        int x = 1280 + i * 120; // separación horizontal entre soldados

        bool enPlataforma = QRandomGenerator::global()->bounded(0, 100) < 70; // 35% probabilidad

        int y = 450; // Suelo por defecto

        if (enPlataforma && !plataformas.isEmpty()) {
            QVector<QGraphicsPixmapItem*> plataformasVisibles;
            for (auto p : plataformas) {
                if (p->x() > 50 && p->x() + p->pixmap().width() < 1280) {  // if (p->x() > 100 && p->x() < 1024)
                    plataformasVisibles.append(p);
                }
            }

            if (!plataformasVisibles.isEmpty()) {
                QGraphicsPixmapItem* plataforma = plataformasVisibles[QRandomGenerator::global()->bounded(0, plataformasVisibles.size())];
                y = plataforma->y() - 100;  // ajustar altura para que el enemigo quede sobre la plataforma
                x = plataforma->x() + QRandomGenerator::global()->bounded(0, plataforma->pixmap().width() - 100);
            }
        }

        Enemigo* enemigo = new Enemigo(x, y, 100, 100, goku);
        enemigo->setPos(x, y);
        escena->addItem(enemigo);
        enemigos.append(enemigo);
    }
}

void Juego::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) {
        goku->saltar();
        goku->cayendoLento = true;
    }
    if (event->key() == Qt::Key_S) {
        if (gokuEnCuerda) {
            // Soltar cuerda
            gokuEnCuerda = false;
            goku->setVisible(true);
            goku->setX(gokuColgado->x());
            goku->setY(gokuColgado->y());
            goku->activarCaida();
            gokuColgado->setVisible(false);
        } else {
            // Caída normal
            goku->acelerarCaida();
            goku->forzarCaida();
        }
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
    int cantidad = QRandomGenerator::global()->bounded(1, 3); // 1 o 2 plataformas
    int baseX = 1280;

    QVector<int> alturas = {450, 295};

    for (int i = 0; i < cantidad; ++i) {
        // Selecciona imagen de plataforma ya cargada
        QPixmap plataformaSprite = imagenesPlataformas[QRandomGenerator::global()->bounded(imagenesPlataformas.size())];
        QGraphicsPixmapItem* plataforma = new QGraphicsPixmapItem(plataformaSprite);
        plataforma->setZValue(1);

        // Altura fija dependiendo de si hay 1 o 2
        int y;
        if (cantidad == 2) {
            y = alturas[i];  // altura 0 → 250, altura 1 → 400
        } else {
            y = alturas[QRandomGenerator::global()->bounded(alturas.size())];  // aleatorio entre 250 y 400
        }

        int posX = baseX + i * 650;  // buena separación cuando hay 2
        plataforma->setPos(posX, y);
        escena->addItem(plataforma);
        plataformas.append(plataforma);

        // --- Posible obstáculo encima ---
        bool colocarObstaculo = QRandomGenerator::global()->bounded(0, 100) < 60;  // 60% chance

        if (colocarObstaculo && !imagenesTroncos.isEmpty() && !imagenesRocas.isEmpty()) {
            QVector<QPixmap>& fuente = (QRandomGenerator::global()->bounded(0, 2) == 0) ? imagenesTroncos : imagenesRocas;
            QPixmap obstaculoSprite = fuente[QRandomGenerator::global()->bounded(fuente.size())];
            QGraphicsPixmapItem* obstaculo = new QGraphicsPixmapItem(obstaculoSprite);
            obstaculo->setZValue(2);

            // Posición: centro, izquierda o derecha
            int opcion = QRandomGenerator::global()->bounded(0, 2); // 0 centro, 1 izq, 2 der
            int offsetX = 0;
            if (opcion == 1) offsetX = plataforma->pixmap().width() - obstaculoSprite.width();
            else offsetX = (plataforma->pixmap().width() - obstaculoSprite.width()) / 2;

            int obstX = plataforma->x() + offsetX;
            int obstY = plataforma->y() + plataforma->pixmap().height() - obstaculoSprite.height() + 0;

            obstaculo->setPos(obstX, obstY);
            escena->addItem(obstaculo);
            obstaculos.append(obstaculo);
        }
    }
}

void Juego::generarObstaculo() {
    if (QRandomGenerator::global()->bounded(0, 100) > 50) return;
    bool esTronco = QRandomGenerator::global()->bounded(0, 2) == 0;
    QVector<QPixmap>& fuente = esTronco ? imagenesTroncos : imagenesRocas;

    QPixmap sprite = fuente[QRandomGenerator::global()->bounded(fuente.size())];
    auto obst = new QGraphicsPixmapItem(sprite);
    obst->setZValue(1);

    int posX = 1280 + (obstaculos.size() % 2 == 0 ? 0 : 80);
    int posY = 680 - sprite.height();

    obst->setPos(posX, posY);
    escena->addItem(obst);
    obstaculos.append(obst);
}

void Juego::actualizarCuerda() {
    // Física del péndulo
    double ax = -gravedad * sin(angulo);
    velocidadAngular += ax;
    angulo += velocidadAngular;

    // Limitar el ángulo
    const double minAng = -M_PI / 4;
    const double maxAng =  M_PI / 4;
    if (angulo < minAng) {
        angulo = minAng;
        velocidadAngular *= -0.8;
    } else if (angulo > maxAng) {
        angulo = maxAng;
        velocidadAngular *= -0.8;
    }

    // Posición del extremo
    QPointF extremo(
        origenCuerda.x() + largoCuerda * sin(angulo),
        origenCuerda.y() + largoCuerda * cos(angulo)
        );

    // Mover cuerda visual
    cuerdaLine->setLine(origenCuerda.x(), origenCuerda.y(), extremo.x(), extremo.y());

    // Si Goku está colgado, lo sigue
    if (gokuEnCuerda && gokuColgado) {
        gokuColgado->setPos(extremo.x() - gokuColgado->pixmap().width() / 2,
                            extremo.y());
    }
}

void Juego::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) {
        goku->cayendoLento = false;
    }
}
