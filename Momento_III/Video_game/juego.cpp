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

    // Actualiza las cuerdas que ya están
    timerAnimacionCuerda = new QTimer(this);
    connect(timerAnimacionCuerda, &QTimer::timeout, this, &Juego::actualizarCuerda);
    timerAnimacionCuerda->start(16);  // 60 FPS

    // Genera nuevas cuerdas aleatorias
    timerGeneracionCuerdas = new QTimer(this);
    connect(timerGeneracionCuerdas, &QTimer::timeout, this, &Juego::generarCuerda);
    timerGeneracionCuerdas->start(6000);

    // Inicialización robusta del tronco
    troncoActual.sprite = nullptr;
    troncoActual.enSuelo = false;

    timerTroncos = new QTimer(this);
    connect(timerTroncos, &QTimer::timeout, this, &Juego::actualizarTronco);
    timerTroncos->start(16);

    // Generar primer tronco con delay inicial
    QTimer::singleShot(2000, this, &Juego::generarTroncoUnico); // Espera 2 segundos al inicio
}

void Juego::iniciar() {
    timerJuego->start(16);         // ~60 FPS
    timerEnemigos->start(7000);    // enemigos cada 1.5 s
    generarTroncoUnico(); // Primer tronco
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

    //GOKU CON  LA CUERDA
    // Detección de colisión con cuerdas
    if (!gokuEnCuerda && goku->y() < 300) { // Solo verificar si está en altura de cuerdas
        for (Cuerda& cuerda : cuerdas) {
            if (!cuerda.activa) {
                QPointF extremo(
                    cuerda.origen.x() + cuerda.largo * sin(cuerda.angulo),
                    cuerda.origen.y() + cuerda.largo * cos(cuerda.angulo)
                    );

                // Verificar colisión con Goku (mejor detección)
                QRectF areaCuerda(extremo.x() - 30, extremo.y() - 30, 60, 60);
                if (goku->collidesWithItem(cuerda.cuerdaItem) ||
                    areaCuerda.contains(goku->pos())) {

                    cuerda.activa = true;
                    cuerda.gokuAgarrado = true;
                    gokuEnCuerda = true;

                    // Ocultar Goku normal y mostrar sprite en cuerda
                    goku->setVisible(false);
                    if (cuerda.gokuSprite) {
                        cuerda.gokuSprite->setVisible(true);
                        cuerda.gokuSprite->setPos(
                            extremo.x() - cuerda.gokuSprite->pixmap().width()/2,
                            extremo.y() - 20
                            );
                    }
                    break;
                }
            }
        }
    }

    // Dentro de Juego::actualizar(): TRONCO
    //if (!troncosGiratorios.isEmpty() && QRandomGenerator::global()->bounded(0, 100) > 95) {
//        actualizarTroncosGiratorios();
  //  }
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

void Juego::generarCuerda() {
    if (QRandomGenerator::global()->bounded(0, 100) > 50) return;

    Cuerda c;
    // Posición inicial (esquina superior derecha con variación)
    c.origen = QPointF(width() + QRandomGenerator::global()->bounded(50, 200),
                       QRandomGenerator::global()->bounded(50, 150));

    // Configuración física
    c.largo = 220; //+ QRandomGenerator::global()->bounded(-30, 30); // Longitud variable
    c.angulo = -M_PI/4; // Ángulo inicial fijo
    c.velocidadAngular = 0;
    c.activa = false;
    c.gokuAgarrado = false;

    // Crear curva Bézier para la cuerda
    QPointF extremo = calcularExtremo(c);
    c.puntoMedio = QPointF((c.origen.x() + extremo.x())/2,
                           (c.origen.y() + extremo.y())/2 + 25); // Curvatura inicial

    QPainterPath path;
    path.moveTo(c.origen);
    path.quadTo(c.puntoMedio, extremo);

    // Configuración visual de la cuerda
    c.cuerdaItem = new QGraphicsPathItem(path);
    QPen penCuerda(QColor(160, 82, 45), 4.5); // Color marrón con grosor
    penCuerda.setCapStyle(Qt::RoundCap);
    c.cuerdaItem->setPen(penCuerda);
    c.cuerdaItem->setZValue(1);
    escena->addItem(c.cuerdaItem);

    // Sprite de Goku (preparado pero oculto)
    QPixmap sprite(":/sprites/Pictures/goku_agarrado.png");
    c.gokuSprite = new QGraphicsPixmapItem(sprite.scaled(80, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    c.gokuSprite->setPos(extremo.x() - 40, extremo.y() - 20);
    c.gokuSprite->setVisible(false);
    c.gokuSprite->setZValue(3);
    escena->addItem(c.gokuSprite);

    cuerdas.append(c);
}

void Juego::actualizarCuerda() {
    for (int i = 0; i < cuerdas.size(); ++i) {
        Cuerda& cuerda = cuerdas[i];

        // 1. Mover con scroll
        cuerda.origen.setX(cuerda.origen.x() - velocidadScroll);

        // 2. Física del péndulo (solo si está activa)
        if (cuerda.activa) {
            const double gravedad = 0.7;
            const double amortiguacion = 0.996;
            const double anguloLiberacion = M_PI/3; // 60° exactos

            // Calcular aceleración angular
            double aceleracion = -gravedad / cuerda.largo * sin(cuerda.angulo);
            cuerda.velocidadAngular += aceleracion;
            cuerda.velocidadAngular *= amortiguacion;

            // Verificar si debemos liberar (antes de actualizar el ángulo)
            // Liberación automática más agresiva
            if (cuerda.gokuAgarrado &&
                (abs(cuerda.angulo) >= anguloLiberacion ||
                 abs(cuerda.velocidadAngular) < 0.01)) {
                soltarGokuDeCuerda(cuerda);
                continue;
            }

            // Actualizar ángulo si no se liberó
            cuerda.angulo += cuerda.velocidadAngular;
        }

        // Resto de la función permanece igual...
        QPointF extremo = calcularExtremo(cuerda);

        // Actualizar posición del sprite de Goku si sigue agarrado
        if (cuerda.gokuAgarrado && cuerda.gokuSprite) {
            cuerda.gokuSprite->setPos(extremo.x() - 40, extremo.y() - 10);
        }

        // Actualizar gráficos de la cuerda
        float factorCurvatura = 25 + 15 * sin(cuerda.angulo * 2.5);
        cuerda.puntoMedio = QPointF(
            (cuerda.origen.x() + extremo.x())/2 + 5 * cos(cuerda.angulo),
            (cuerda.origen.y() + extremo.y())/2 + factorCurvatura
            );

        QPainterPath path;
        path.moveTo(cuerda.origen);
        path.quadTo(cuerda.puntoMedio, extremo);
        cuerda.cuerdaItem->setPath(path);

        // Eliminar si sale de pantalla
        if (cuerda.origen.x() + cuerda.largo < 0) {
            escena->removeItem(cuerda.cuerdaItem);
            if (cuerda.gokuSprite) escena->removeItem(cuerda.gokuSprite);
            delete cuerda.cuerdaItem;
            delete cuerda.gokuSprite;
            cuerdas.removeAt(i--);
        }
    }
}

void Juego::activarCuerda(Goku* goku) {
    if (gokuEnCuerda) return;

    for (Cuerda& cuerda : cuerdas) {
        if (!cuerda.activa) {
            QPointF extremo(
                cuerda.origen.x() + cuerda.largo * sin(cuerda.angulo),
                cuerda.origen.y() + cuerda.largo * cos(cuerda.angulo)
                );

            // Verificar colisión con Goku (radio de 50px)
            if (QLineF(goku->pos(), extremo).length() < 50) {
                cuerda.activa = true;
                gokuEnCuerda = true;

                // Ocultar sprite normal de Goku y mostrar el de cuerda
                goku->setVisible(false);
                if (cuerda.gokuSprite) {
                    cuerda.gokuSprite->setVisible(true);
                    cuerda.gokuSprite->setPos(
                        extremo.x() - cuerda.gokuSprite->pixmap().width() / 2,
                        extremo.y() - 20
                        );
                }
                break;
            }
        }
    }
}

void Juego::soltarGokuDeCuerda(Cuerda& cuerda) {
    if (!cuerda.gokuAgarrado) return;

    // Calcular posición final
    QPointF extremo(
        cuerda.origen.x() + cuerda.largo * sin(cuerda.angulo),
        cuerda.origen.y() + cuerda.largo * cos(cuerda.angulo)
        );

    // Restablecer a Goku
    goku->setPos(extremo);
    goku->setVisible(true);
    goku->activarCaida();
    goku->forzarCaida();

    // Restablecer la cuerda
    cuerda.gokuAgarrado = false;
    cuerda.activa = false;
    //cuerda.tiempoEnCuerda = 0;
    if (cuerda.gokuSprite) cuerda.gokuSprite->setVisible(false);

    gokuEnCuerda = false;
}

QPointF Juego::calcularExtremo(const Cuerda& cuerda) const {
    // Cálculo preciso considerando el largo y ángulo actual
    return QPointF(
        cuerda.origen.x() + cuerda.largo * sin(cuerda.angulo),
        cuerda.origen.y() + cuerda.largo * cos(cuerda.angulo)
        );
}

void Juego::generarTroncoUnico() {
    if (troncoActual.sprite != nullptr || !timerJuego->isActive()) {
        qDebug() << "No se puede generar tronco: ya existe uno activo o juego pausado";
        return;
    }

    QPixmap sprite(":/sprites/Pictures/tronco_giratorio.png");
    if (sprite.isNull()) {
        qDebug() << "Error: No se pudo cargar la imagen del tronco";
        return;
    }

    troncoActual.sprite = new QGraphicsPixmapItem(sprite.scaled(80, 80, Qt::KeepAspectRatio));

    // Configuración inicial
    troncoActual.sprite->setPos(1000, QRandomGenerator::global()->bounded(50, 200));
    troncoActual.velocidadY = 2.5;
    troncoActual.velocidadX = 0;
    troncoActual.velocidadRotacion = -3.0;
    troncoActual.rotacionActual = 0;
    troncoActual.enSuelo = false;

    // Configuración gráfica
    troncoActual.sprite->setTransformOriginPoint(
        troncoActual.sprite->boundingRect().center()
        );
    troncoActual.sprite->setZValue(3);

    escena->addItem(troncoActual.sprite);
    qDebug() << "Nuevo tronco generado en posición:" << troncoActual.sprite->pos();
}

void Juego::actualizarTronco() {
    if (troncoActual.sprite == nullptr) return;

    // 1. Rotación continua
    troncoActual.rotacionActual += troncoActual.velocidadRotacion;
    troncoActual.sprite->setRotation(troncoActual.rotacionActual);

    // 2. Movimiento vertical/horizontal
    if (!troncoActual.enSuelo) {
        troncoActual.sprite->moveBy(0, troncoActual.velocidadY);

        // Detección de plataforma o suelo
        QGraphicsPixmapItem* plataforma = detectarPlataformaSobre(troncoActual);

        if (plataforma != nullptr) {
            // Ajustar posición exacta sobre la plataforma
            qreal nuevaY = plataforma->y() - troncoActual.sprite->boundingRect().height();
            troncoActual.sprite->setY(nuevaY);
            troncoActual.enSuelo = true;
            troncoActual.velocidadX = -9.0;
            troncoActual.velocidadY = 0;
        }
        else if (troncoActual.sprite->y() >= 670 - troncoActual.sprite->boundingRect().height()) {
            // Caída al suelo principal
            troncoActual.sprite->setY(670 - troncoActual.sprite->boundingRect().height());
            troncoActual.enSuelo = true;
            troncoActual.velocidadX = -4.0;
            troncoActual.velocidadY = 0;
        }
    }
    else {
        // Movimiento horizontal
        troncoActual.sprite->moveBy(troncoActual.velocidadX, 0);

        // Verificación mejorada para caída desde plataformas
        bool sigueEnPlataforma = false;
        QGraphicsPixmapItem* plataformaActual = detectarPlataformaSobre(troncoActual);

        if (plataformaActual) {
            // Verificar si el tronco sigue completamente sobre la plataforma
            QRectF areaTronco = troncoActual.sprite->boundingRect().translated(troncoActual.sprite->pos());
            QRectF areaPlataforma = plataformaActual->boundingRect().translated(plataformaActual->pos());

            // Margen de 5px para evitar falsos positivos en bordes
            if (areaTronco.left() >= areaPlataforma.left() - 5 &&
                areaTronco.right() <= areaPlataforma.right() + 5) {
                sigueEnPlataforma = true;
            }
        }

        if (!sigueEnPlataforma && troncoActual.sprite->y() < 670 - troncoActual.sprite->boundingRect().height()) {
            troncoActual.enSuelo = false;
            troncoActual.velocidadY = 2.5;
            troncoActual.velocidadX = 0; // Resetear velocidad horizontal al caer
        }
    }

    // 3. Detección de colisión con Goku (PRIMERO verifica esto)
    if (troncoActual.sprite->collidesWithItem(goku)) {
        goku->animarCaida();
        qDebug() << "Goku golpeado por el tronco";

        // Eliminar tronco y programar nuevo
        escena->removeItem(troncoActual.sprite);
        delete troncoActual.sprite;
        troncoActual.sprite = nullptr;
        QTimer::singleShot(3000, this, &Juego::generarTroncoUnico);
        return; // ¡IMPORTANTE! Salir para evitar doble eliminación
    }

    // 4. Eliminar si sale por la izquierda (solo si no hubo colisión con Goku)
    if (troncoActual.sprite->x() < -100) {
        escena->removeItem(troncoActual.sprite);
        delete troncoActual.sprite;
        troncoActual.sprite = nullptr;
        QTimer::singleShot(QRandomGenerator::global()->bounded(3000, 5000),
                           this, &Juego::generarTroncoUnico);
    }
}

void Juego::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) {
        goku->saltar();
        goku->cayendoLento = true;
    }
    if (event->key() == Qt::Key_S) {
        if (gokuEnCuerda) {  // Solo si está en una cuerda
            for (Cuerda& cuerda : cuerdas) {
                if (cuerda.gokuAgarrado) {
                    soltarGokuDeCuerda(cuerda);
                    qDebug() << "Goku se soltó de la cuerda con S";  // Debug
                    break;
                }
            }
        } else {
            // Comportamiento normal de la tecla S (si lo necesitas)
            goku->acelerarCaida();
        }
    }
    if (event->key() == Qt::Key_Space) {
        if (goku->puedeDisparar()) {
            goku->animarDisparo();
            goku->setListaEnemigos(&enemigos);
            goku->disparar(escena);

            goku->reiniciarEnergia();
            actualizarBarraEnergia();
        }
    }
}

void Juego::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_W) {
        goku->cayendoLento = false;
    }
}
