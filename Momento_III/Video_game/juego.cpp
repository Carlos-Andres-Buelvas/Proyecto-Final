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

        for (int i = 0; i < cantidad; ++i) {
            QPixmap sprite(":/sprites/Pictures/capsula.png");
            auto capsula = new QGraphicsPixmapItem(sprite.scaled(40, 40));
            capsula->setZValue(2);

            // Decide si va en el suelo o en una plataforma
            bool enPlataforma = QRandomGenerator::global()->bounded(0, 100) < 35; // 35% de probabilidad de que esté en plataforma

            int posY = 375; // suelo por defecto
            if (enPlataforma && !plataformas.isEmpty()) {
                // Elegir plataforma aleatoria visible
                QVector<QGraphicsPixmapItem*> plataformasVisibles;
                for (auto p : plataformas) {
                    if (p->x() > 200 && p->x() < 1024) {
                        plataformasVisibles.append(p);
                    }
                }

                if (!plataformasVisibles.isEmpty()) {
                    auto plataforma = plataformasVisibles[QRandomGenerator::global()->bounded(0, plataformasVisibles.size())];
                    posY = plataforma->y() - 40;  // encima de la plataforma
                }
            }

            int separacion = i * 45;
            capsula->setPos(1024 + separacion, posY);
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
    timerObstaculos->start(3000);
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
        // ¿Física soldados?
        for (auto plataforma : plataformas) {
            if (enemigo->collidesWithItem(plataforma)) {
                qreal baseY = enemigo->y() + enemigo->boundingRect().height();
                qreal plataformaY = plataforma->y();

                if (baseY <= plataformaY + 15) {
                    // Lo reposicionas justo encima
                    enemigo->setY(plataformaY - enemigo->boundingRect().height());
                    break; // Solo con la primera plataforma que colisiona
                }
            }
        }

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

    // Colisión Goku vs obstáculo
    for (auto o : obstaculos) {
        if (goku->collidesWithItem(o)) {
            goku->animarCaida();
            qDebug() << "Goku ha chocado contra un obstáculo.";
            return;
        }
    }
}

void Juego::generarEnemigo() {
    int cantidad = QRandomGenerator::global()->bounded(1, 3); // 1 o 2 soldados

    for (int i = 0; i < cantidad; ++i) {
        int x = 1024 + i * 120; // separación horizontal entre soldados

        bool enPlataforma = QRandomGenerator::global()->bounded(0, 100) < 35; // 35% probabilidad

        int y = 375; // Suelo por defecto

        if (enPlataforma && !plataformas.isEmpty()) {
            QVector<QGraphicsPixmapItem*> plataformasVisibles;
            for (auto p : plataformas) {
                if (p->x() > 50 && p->x() + p->pixmap().width() < 1024) {  // if (p->x() > 100 && p->x() < 1024)
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
        //enemigo->setPos(x, y);
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
            y = (i == 0) ? 400 : 225;
        } else {
            // Si solo hay una, se escoge aleatoriamente
            y = (QRandomGenerator::global()->bounded(0, 2) == 0) ? 425 : 250;
        }

        plataforma->setPos(baseX + i * 400, y);  // separadas horizontalmente
        escena->addItem(plataforma);
        plataformas.append(plataforma);
    }
}

void Juego::generarObstaculo() {
    // Elegimos qué tipo sale: 0 = tronco, 1 = roca
    int tipo = QRandomGenerator::global()->bounded(0, 2);

    // Cargamos el sprite-sheet apropiado
    QPixmap sheet;
    int cols = 0, filas = 0, fw = 0, fh = 0;
    if (tipo == 0) {
        sheet = QPixmap(":/sprites/Pictures/troncos.png");
        cols = 3; filas = 4; fw = 160; fh = 118;
    } else {
        sheet = QPixmap(":/sprites/Pictures/rocas.png");
        cols = 3; filas = 1; fw = 84;  fh = 35;
    }

    // Elegimos un frame aleatorio
    int totalFrames = cols * filas;
    int idx = QRandomGenerator::global()->bounded(0, totalFrames);
    int cx = (idx % cols) * fw;
    int cy = (idx / cols) * fh;
    QPixmap sprite = sheet.copy(cx, cy, fw, fh);

    // Escalamos un poco para que encaje en tu escena
    //int anchoVisual = fw * 0.75;   // 75% del ancho original, ajústalo
    //int altoVisual  = fh * 0.75;   // idem
    sprite = sprite.scaled(fw, fh, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Creamos el ítem
    auto obst = new QGraphicsPixmapItem(sprite);
    obst->setZValue(1);

    //Posición fija sobre el suelo
    int posX = 1024;
    int posY = 500;
/*
    // Decidir si va en suelo o en plataforma
    bool enPlataforma = QRandomGenerator::global()->bounded(0, 100) < 30  // 30% prob.
                        && !plataformas.isEmpty();

    int posX = 1024;            // inicial justo a la derecha
    int posY = 375 - 0; // suelo

    if (enPlataforma) {
        // Filtramos plataformas visibles
        QVector<QGraphicsPixmapItem*> candidatas;
        for (auto p : plataformas) {
            if (p->x() + p->pixmap().width() > 0 && p->x() < 1024) {
                candidatas.append(p);
            }
        }
        if (!candidatas.isEmpty()) {
            // Elegimos una al azar
            auto plat = candidatas.at(QRandomGenerator::global()->bounded(candidatas.size()));
            posY = plat->y() - altoVisual;
            // Para no tapar al jugador, lo hacemos aparecer centrado en la plataforma
            int maxXoffset = plat->pixmap().width() - anchoVisual;
            posX = int(plat->x()) + QRandomGenerator::global()->bounded(0, maxXoffset);
        }
    }
*/
    // Desplazamos horizontalmente según cuántos obstáculos haya
    int separacion = obstaculos.size() % 2 == 0 ? 0 : 80;
    obst->setPos(posX + separacion, posY);

    escena->addItem(obst);
    obstaculos.append(obst);
}

void Juego::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) {
        goku->cayendoLento = false;
    }
}
