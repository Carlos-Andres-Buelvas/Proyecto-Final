#include "juego.h"
#include "enemigo.h"
#include "mainwindow.h"
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QRandomGenerator>
#include <QDebug>
#include <QGraphicsScene>
#include <QPixmap>

#include <QFontDatabase>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsColorizeEffect>
#include <QGraphicsItemAnimation>
#include <QGraphicsProxyWidget>
#include <QMediaPlayer>
#include <QAudioOutput>

Juego::Juego(QWidget *parent) : QGraphicsView(parent), timerAnimacionCuerda(nullptr),
    timerGeneracionCuerdas(nullptr), btnContinuar(nullptr), btnSalir(nullptr),
    proxyContinuar(nullptr), proxySalir(nullptr), pajaroItem(nullptr), frameActualPajaro(0), timerAnimacionPajaro(nullptr) {
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 1280, 680);
    setScene(escena);
    setFixedSize(1280, 680);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::StrongFocus);

    // Cargar fuente Dragon Ball con manejo de errores
    int fontId = QFontDatabase::addApplicationFont(":/fondos/Pictures/db_font.ttf");
    if (fontId == -1) {
        qDebug() << "Error al cargar la fuente Dragon Ball";
        dragonBallFont = "Arial"; // Usar fuente por defecto
    } else {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            dragonBallFont = fontFamilies.at(0);
        } else {
            qDebug() << "No se encontraron familias de fuentes";
            dragonBallFont = "Arial";
        }
    }

    // Guardar la fuente como miembro de clase
    this->dragonBallFont = dragonBallFont;

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
        fondo->setPos(i * (fondoPixmap.width() + 5), 0);
        fondo->setZValue(-1);
        escena->addItem(fondo);
        fondosScroll.append(fondo);
    }

    // Crear Goku
    goku = new Goku(0, 450, 275, 275);
    goku->setZValue(2);
    escena->addItem(goku);

    // Timer principal del juego
    timerJuego = new QTimer(this);
    connect(timerJuego, &QTimer::timeout, this, &Juego::actualizar);

    // Timer para generar enemigos
    timerEnemigos = new QTimer(this);
    connect(timerEnemigos, &QTimer::timeout, this, &Juego::generarEnemigo);

    // Barra de energía
    barraEnergia = new QGraphicsRectItem(25, 25, 0, 20);
    QLinearGradient gradient(0, 0, 200, 0);
    gradient.setColorAt(0, QColor(255, 255, 0));
    gradient.setColorAt(0.5, QColor(255, 165, 0));
    gradient.setColorAt(1, QColor(255, 0, 0));
    barraEnergia->setBrush(QBrush(gradient));
    barraEnergia->setPen(QPen(QColor(255, 215, 0), 2));
    barraEnergia->setZValue(11);
    escena->addItem(barraEnergia);

    // Texto "ENERGÍA"
    QGraphicsTextItem* energiaText = new QGraphicsTextItem("ENERGIA");
    energiaText->setDefaultTextColor(QColor(255, 215, 0));
    energiaText->setFont(QFont(dragonBallFont, 16));
    energiaText->setPos(30, 0);

    QGraphicsDropShadowEffect* textShadow = new QGraphicsDropShadowEffect();
    textShadow->setBlurRadius(5);
    textShadow->setColor(QColor(0, 0, 0, 200));
    textShadow->setOffset(3, 3);
    energiaText->setGraphicsEffect(textShadow);

    energiaText->setZValue(12);
    escena->addItem(energiaText);

    // Contador de soldados
    contadorSoldados = new QGraphicsTextItem();
    contadorSoldados->setHtml(
        "<div style='color: #ffcc00; font-family: \"" + dragonBallFont + "\"; font-size: 18px; "
                                                                         "font-weight: bold; text-shadow: 3px 3px 5px #000000; letter-spacing: 2px;'>"
                                                                         "ENEMIGOS: <span style='color: #ffffff;'>0</span>/" + QString::number(OBJETIVO_SOLDADOS) +
        "</div>"
        );
    contadorSoldados->setPos(30, 65);
    contadorSoldados->setZValue(12);
    escena->addItem(contadorSoldados);

    // Botón de pausa
    botonPausa = new QPushButton(this);
    botonPausa->setGeometry(width() - 150, 20, 120, 50);
    botonPausa->setIconSize(QSize(120, 50));
    botonPausa->setText("PAUSA");
    botonPausa->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   color: #ffcc00;"
        "   font: bold 18px '" + dragonBallFont + "';"
                           "   padding: 0px;"
                           "   text-shadow: 2px 2px 4px #000000;"
                           "}"
                           "QPushButton:hover {"
                           "   color: #ffffff;"
                           "}"
                           "QPushButton:pressed {"
                           "   color: #ff9900;"
                           "   padding-top: 2px;"
                           "   padding-left: 2px;"
                           "}"
        );

    QGraphicsDropShadowEffect* buttonShadow = new QGraphicsDropShadowEffect();
    buttonShadow->setBlurRadius(10);
    buttonShadow->setColor(QColor(255, 165, 0, 150));
    buttonShadow->setOffset(5, 5);
    botonPausa->setGraphicsEffect(buttonShadow);

    connect(botonPausa, &QPushButton::clicked, this, &Juego::togglePausa);

    // Timer de cápsulas
    timerCapsulas = new QTimer(this);
    connect(timerCapsulas, &QTimer::timeout, this, [=]() {
        int cantidad = QRandomGenerator::global()->bounded(1, 4);  // 1 a 3 cápsulas

        QVector<QGraphicsPixmapItem*> plataformasVisibles;
        for (auto p : plataformas) {
            if (p->x() > 100 && p->x() + p->pixmap().width() < 1280) {
                plataformasVisibles.append(p);
            }
        }

        for (int i = 0; i < cantidad; ++i) {
            QPixmap sprite(":/sprites/Pictures/capsula.png");
            auto capsula = new QGraphicsPixmapItem(sprite.scaled(40, 40));
            capsula->setZValue(2);

            int posX = 1280 + i * 45;
            int posY = 550;  // por defecto, en el suelo

            // Si hay plataformas disponibles, poner cápsulas encima de alguna de ellas
            bool enPlataforma = QRandomGenerator::global()->bounded(0, 100) < 50;
            if (enPlataforma && !plataformasVisibles.isEmpty()) {
                auto plataforma = plataformasVisibles.at(QRandomGenerator::global()->bounded(plataformasVisibles.size()));
                posY = plataforma->y() - 40;
                posX = plataforma->x() + QRandomGenerator::global()->bounded(0, plataforma->pixmap().width() - 40);
            }

            capsula->setPos(posX, posY);
            escena->addItem(capsula);
            capsulas.append(capsula);
        }
    });
    timerCapsulas->start(4500);

    timerPlataformas = new QTimer(this);
    connect(timerPlataformas, &QTimer::timeout, this, &Juego::generarPlataforma);
    timerPlataformas->start(7000);

    timerObstaculos = new QTimer(this);
    connect(timerObstaculos, &QTimer::timeout, this, &Juego::generarObstaculo);
    timerObstaculos->start(7000);

    timerAnimacionCuerda = new QTimer(this);
    connect(timerAnimacionCuerda, &QTimer::timeout, this, &Juego::actualizarCuerda);
    timerAnimacionCuerda->start(16);

    timerGeneracionCuerdas = new QTimer(this);
    connect(timerGeneracionCuerdas, &QTimer::timeout, this, &Juego::generarCuerda);
    timerGeneracionCuerdas->start(6000);

    troncoActual.sprite = nullptr;
    troncoActual.enSuelo = false;

    timerTroncos = new QTimer(this);
    connect(timerTroncos, &QTimer::timeout, this, &Juego::actualizarTronco);
    timerTroncos->start(16);

    QTimer::singleShot(7000, this, &Juego::generarTroncoUnico);

    cargarAssetsDecoracion();
    generarDecoracionPalmeras();

    // Iniciar animación del pájaro con un pequeño retraso aleatorio
    QTimer::singleShot(QRandomGenerator::global()->bounded(1000, 3000),
                       this, &Juego::iniciarAnimacionPajaro);

    // Timer para decoración más eficiente
    QTimer* timerDecoracion = new QTimer(this);
    connect(timerDecoracion, &QTimer::timeout, this, &Juego::actualizarDecoracion);
    timerDecoracion->start(16);  // ~60 FPS para movimiento suave

    timerGameOver = new QTimer(this);
    timerGameOver->setSingleShot(true);
    connect(timerGameOver, &QTimer::timeout, this, &Juego::mostrarMenuGameOver);

    this->show();
    this->setFocus();
}

void Juego::iniciar() {
    timerJuego->start(16);         // ~60 FPS
    int intervalo = QRandomGenerator::global()->bounded(10000, 15000); // Entre 10 y 15 segundos
    timerEnemigos->start(intervalo);
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

    // Asegurar que Goku no se quede atrás
    if (goku->x() < 700) {
        goku->setX(goku->x() + velocidadScroll * 2);
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
            mostrarGameOver();
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
                mostrarGameOver();
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
            qDebug() << std::abs(obstY - gokuY);

            // ⚠️ Tolerancia para colisión en el suelo
            if (std::abs(obstY - gokuY) < 390) {
                goku->animarCaida();
                mostrarGameOver();
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
                    mostrarGameOver();
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
}

void Juego::generarEnemigo() {
    if (enemigos.size() >= 2) {
        return; // No generar más si ya hay 2 en pantalla
    }
    // Generar 1 o 2 soldados aleatoriamente (75% de probabilidad para cada caso)
    int cantidad = (QRandomGenerator::global()->bounded(0, 100) < 75) ? 1 : 2;

    for (int i = 0; i < cantidad; ++i) {
        // Posición X: aparecen fuera de pantalla a la derecha con separación
        int x = 1280 + i * 200; // 150px de separación entre soldados

        // Posición Y fija en el suelo (ajusta este valor según tu escenario)
        int y = 510; // Valor de Y para el suelo (ajusta según necesites)

        // Crear el enemigo
        Enemigo* enemigo = new Enemigo(x, y, 100, 100, goku, false);
        enemigo->setPos(x, y);
        escena->addItem(enemigo);
        enemigos.append(enemigo);

        qDebug() << "Soldado generado en posición:" << x << "," << y;
    }
}

void Juego::actualizarBarraEnergia() {
    float porcentaje = static_cast<float>(goku->obtenerEnergia()) / 100.0f;
    QRectF targetRect(25, 25, porcentaje * 200, 20);

    // Crear la animación
    QTimeLine *timer = new QTimeLine(200, this); // 200 ms de duración
    timer->setFrameRange(0, 100);

    // Conectar la señal frameChanged para actualizar manualmente el rectángulo
    connect(timer, &QTimeLine::frameChanged, [this, timer, targetRect](int frame) {
        QRectF currentRect = barraEnergia->rect();
        qreal progress = timer->currentValue();

        // Interpolar entre el rectángulo actual y el objetivo
        QRectF interpolatedRect(
            currentRect.x(),
            currentRect.y(),
            currentRect.width() + (targetRect.width() - currentRect.width()) * progress,
            currentRect.height()
            );

        barraEnergia->setRect(interpolatedRect);
    });

    // Eliminar el timer cuando termine
    connect(timer, &QTimeLine::finished, timer, &QTimeLine::deleteLater);

    // Iniciar la animación
    timer->start();

    // Efecto especial cuando está llena
    if (goku->puedeDisparar()) {
        QGraphicsColorizeEffect* colorize = new QGraphicsColorizeEffect(this);
        colorize->setColor(QColor(255, 255, 255, 150));
        barraEnergia->setGraphicsEffect(colorize);

        QTimer::singleShot(500, [this]() {
            barraEnergia->setGraphicsEffect(nullptr);
        });
    }
}

void Juego::aumentarContadorSoldados() {
    soldadosEliminados++;
    contadorSoldados->setHtml(
        "<div style='color: #ffcc00; font-family: \"" + dragonBallFont + "\"; font-size: 18px; "
                                                                         "font-weight: bold; text-shadow: 3px 3px 5px #000000; letter-spacing: 2px;'>"
                                                                         "ENEMIGOS: <span style='color: #ffffff;'>" + QString::number(soldadosEliminados) + "</span>/" +
        QString::number(OBJETIVO_SOLDADOS) + "</div>"
        );

    if (soldadosEliminados >= OBJETIVO_SOLDADOS) {
        detenerTodo();
        QGraphicsTextItem* textoNivelCompletado = new QGraphicsTextItem("NIVEL COMPLETADO");
        textoNivelCompletado->setDefaultTextColor(QColor(255, 215, 0));
        textoNivelCompletado->setFont(QFont(dragonBallFont, 36));
        textoNivelCompletado->setPos(width()/2 - textoNivelCompletado->boundingRect().width()/2, height()/2 - 50);

        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(10);
        shadow->setColor(Qt::black);
        shadow->setOffset(5, 5);
        textoNivelCompletado->setGraphicsEffect(shadow);

        textoNivelCompletado->setZValue(1001);
        escena->addItem(textoNivelCompletado);

        // Emitir señal después de un breve retraso
        QTimer::singleShot(3000, this, [this]() {
            emit nivelCompletado();
            detenerTodo();
        });
    }
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
            obstaculo->setZValue(1);

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

    c.origen = QPointF(1280, -5);
    // Configuración física
    c.largo = 215;
    c.angulo = -M_PI/4; // Ángulo inicial fijo
    c.velocidadAngular = 0;
    c.activa = false;
    c.gokuAgarrado = false;

    // Crear curva Bézier para la cuerda
    QPointF extremo = calcularExtremo(c);
    c.puntoMedio = QPointF((c.origen.x() + extremo.x())/2,
                           (c.origen.y() + extremo.y())/2 + 50); // Curvatura inicial

    QPainterPath path;
    path.moveTo(c.origen);
    path.quadTo(c.puntoMedio, extremo);

    // Configuración visual de la cuerda
    c.cuerdaItem = new QGraphicsPathItem(path);
    QPen penCuerda(QColor(160, 82, 45), 8); // Color marrón con grosor
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
        float factorCurvatura = 25 + 15 * sin(cuerda.angulo * 5.5);
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
    // Solución: Ajustar la posición X manualmente
    extremo.setX(extremo.x() + velocidadScroll + 10);
    goku->setVisible(true);
    //goku->activarCaida();
    //goku->forzarCaida();

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
    troncoActual.sprite->setPos(1200, QRandomGenerator::global()->bounded(0, 20));
    troncoActual.velocidadY = 3.0;
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
            troncoActual.velocidadX = -4.5;
            troncoActual.velocidadY = 0;
        }
        else if (troncoActual.sprite->y() >= 670 - troncoActual.sprite->boundingRect().height()) {
            // Caída al suelo principal
            troncoActual.sprite->setY(670 - troncoActual.sprite->boundingRect().height());
            troncoActual.enSuelo = true;
            troncoActual.velocidadX = -9.0;
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
        mostrarGameOver();
        qDebug() << "Goku golpeado por el tronco";

        // Eliminar tronco y programar nuevo
        escena->removeItem(troncoActual.sprite);
        delete troncoActual.sprite;
        troncoActual.sprite = nullptr;
        QTimer::singleShot(7000, this, &Juego::generarTroncoUnico);
        return; // ¡IMPORTANTE! Salir para evitar doble eliminación
    }

    // 4. Eliminar si sale por la izquierda (solo si no hubo colisión con Goku)
    if (troncoActual.sprite->x() < -100) {
        escena->removeItem(troncoActual.sprite);
        delete troncoActual.sprite;
        troncoActual.sprite = nullptr;
        QTimer::singleShot(7000, this, &Juego::generarTroncoUnico);
    }
}

void Juego::keyPressEvent(QKeyEvent* event) {
    if (pausado) {
        // Solo permitir despausar con ESPACIO cuando está pausado
        if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
            togglePausa();
        }
        return;
    }

    // Comportamiento normal del juego
    switch (event->key()) {
    case Qt::Key_W:
        goku->saltar();
        goku->cayendoLento = true;
        break;

    case Qt::Key_S:
        if (gokuEnCuerda) {
            for (Cuerda& cuerda : cuerdas) {
                if (cuerda.gokuAgarrado) {
                    soltarGokuDeCuerda(cuerda);
                    break;
                }
            }
        } else {
            goku->acelerarCaida();
        }
        break;

    case Qt::Key_P:
        if (!pPresionado && goku->puedeDisparar()) {
            pPresionado = true;
            goku->animarDisparo();
            goku->setListaEnemigos(&enemigos);
            goku->disparar(escena);
            goku->reiniciarEnergia();
            actualizarBarraEnergia();
        }
        break;

    case Qt::Key_Space:
        if (!event->isAutoRepeat()) {
            togglePausa();
        }
        break;
    }
}

void Juego::keyReleaseEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_W:
        goku->cayendoLento = false;
        break;

    case Qt::Key_P:
        pPresionado = false;
        break;
    }
    QGraphicsView::keyReleaseEvent(event);
}

void Juego::agregarItemEscena(QGraphicsItem* item) {
    escena->addItem(item);
}

void Juego::removerItemEscena(QGraphicsItem* item) {
    escena->removeItem(item);
}

void Juego::detenerTodo() {
    // Detener todos los timers
    timerJuego->stop();
    timerEnemigos->stop();
    timerAnimacionCuerda->stop();
    timerGeneracionCuerdas->stop();
    timerPlataformas->stop();
    timerObstaculos->stop();
    timerTroncos->stop();
    timerCapsulas->stop();

    // Pausar todos los enemigos y sus proyectiles
    for (Enemigo* enemigo : enemigos) {
        enemigo->setPausado(true);
    }
}

void Juego::reanudarTodo() {
    // Reanudar todos los timers
    timerJuego->start(16);
    timerEnemigos->start(10000);
    timerAnimacionCuerda->start(16);
    timerGeneracionCuerdas->start(6000);
    timerPlataformas->start(7000);
    timerObstaculos->start(7000);
    timerTroncos->start(16);
    timerCapsulas->start(3000);

    // Reanudar enemigos y proyectiles
    for (Enemigo* enemigo : enemigos) {
        enemigo->setPausado(false);
    }
}

void Juego::togglePausa() {
    pausado = !pausado;

    if (pausado) {
        detenerTodo();

        // Fondo semitransparente (como ya lo tienes)
        QGraphicsRectItem* overlay = new QGraphicsRectItem(0, 0, width(), height());
        overlay->setBrush(QColor(0, 0, 0, 180));
        overlay->setZValue(999);
        overlay->setData(0, "pausa_overlay");
        escena->addItem(overlay);

        // Texto "PAUSA" (como ya lo tienes)
        QGraphicsTextItem* pausaText = new QGraphicsTextItem("PAUSE");
        pausaText->setDefaultTextColor(QColor(255, 215, 0));
        pausaText->setFont(QFont(dragonBallFont, 58, QFont::Bold));
        // ... (configuración de sombra y posición)

        // Configurar botones (solo la primera vez)
        if (!btnContinuar) {
            configurarBotonesPausa();
        } else {
            proxyContinuar->show();
            proxySalir->show();
        }

    } else {
        // Limpiar elementos de pausa
        for (QGraphicsItem* item : escena->items()) {
            if (item->data(0).toString() == "pausa_overlay") {
                escena->removeItem(item);
                delete item;
            }
        }

        // Ocultar botones (no eliminarlos para mejor performance)
        if (proxyContinuar) proxyContinuar->hide();
        if (proxySalir) proxySalir->hide();

        reanudarTodo();
    }
}

void Juego::configurarBotonesPausa() {
    // Botón Continuar
    btnContinuar = new QPushButton("CONTINUAR");
    btnContinuar->setObjectName("btnPausa");
    btnContinuar->setFont(QFont(dragonBallFont, 20, QFont::Bold));
    btnContinuar->setStyleSheet(
        "QPushButton#btnPausa {"
        "   background-color: #ff9900;"
        "   color: white;"
        "   border: 3px solid #ffcc00;"
        "   border-radius: 15px;"
        "   padding: 10px 25px;"
        "   min-width: 200px;"
        "}"
        "QPushButton#btnPausa:hover {"
        "   background-color: #ffaa33;"
        "   border-color: #ffffff;"
        "}"
        );

    // Botón Salir
    btnSalir = new QPushButton("SALIR AL MENÚ");
    btnSalir->setObjectName("btnPausa");
    btnSalir->setFont(QFont(dragonBallFont, 20, QFont::Bold));
    btnSalir->setStyleSheet(
        "QPushButton#btnPausa {"
        "   background-color: #cc0000;"
        "   color: white;"
        "   border: 3px solid #ff4444;"
        "   border-radius: 15px;"
        "   padding: 10px 25px;"
        "   min-width: 200px;"
        "}"
        "QPushButton#btnPausa:hover {"
        "   background-color: #ff4444;"
        "   border-color: #ffffff;"
        "}"
        );

    // Posicionamiento usando QGraphicsProxyWidget
    proxyContinuar = escena->addWidget(btnContinuar);
    proxyContinuar->setPos(width()/2 - btnContinuar->width()/2, height()/2 + 30);
    proxyContinuar->setZValue(1002);

    proxySalir = escena->addWidget(btnSalir);
    proxySalir->setPos(width()/2 - btnSalir->width()/2, height()/2 + 110);
    proxySalir->setZValue(1002);

    // Conexiones de señales
    connect(btnContinuar, &QPushButton::clicked, this, &Juego::togglePausa);
    connect(btnSalir, &QPushButton::clicked, this, [this]() {
        emit salirAlMenu();
        togglePausa();
        detenerTodo();
    });
}

void Juego::cargarAssetsDecoracion()
{
    // Cargar palmeras
    spritesPalmeras = {
        QPixmap(":/fondos/Pictures/palmera_1.png"),
        QPixmap(":/fondos/Pictures/palmera_2.png"),
        QPixmap(":/fondos/Pictures/palmera_3.png"),
        QPixmap(":/fondos/Pictures/palmera_4.png")
    };

    // Cargar frames del pájaro
    QPixmap spriteSheetPajaro(":/fondos/Pictures/pajaro.png");
    if (!spriteSheetPajaro.isNull()) {
        framesPajaro.clear();  // Limpiar por si acaso
        for (int fila = 0; fila < 4; ++fila) {
            for (int col = 0; col < 6; ++col) {
                framesPajaro.append(spriteSheetPajaro.copy(col * 200, fila * 180, 200, 180));
            }
        }
    } else {
        qDebug() << "Error: No se pudo cargar el sprite del pájaro";
    }
}

void Juego::generarDecoracionPalmeras()
{
    // Limpiar palmeras existentes
    for (auto palmera : decoracionPalmeras) {
        escena->removeItem(palmera);
        delete palmera;
    }
    decoracionPalmeras.clear();

    // Generar nuevas palmeras (10-15 palmeras)
    int cantidadPalmeras = QRandomGenerator::global()->bounded(5, 9);
    for (int i = 0; i < cantidadPalmeras; ++i) {
        int tipo = QRandomGenerator::global()->bounded(spritesPalmeras.size());
        QGraphicsPixmapItem* palmera = new QGraphicsPixmapItem(spritesPalmeras[tipo]);

        // Tamaño constante (2x el tamaño original)
        qreal escala = 4.5;
        palmera->setScale(escala);

        // Posición aleatoria
        qreal x = 1280 + QRandomGenerator::global()->bounded(4000);  // Más separación
        qreal y = 300 + QRandomGenerator::global()->bounded(150);    // Mismo rango vertical

        palmera->setPos(x, y);
        palmera->setZValue(-1);  // Detrás del fondo principal

        escena->addItem(palmera);
        decoracionPalmeras.append(palmera);
    }
}

void Juego::iniciarAnimacionPajaro()
{
    if (framesPajaro.empty()) return;

    if (pajaroItem) {
        escena->removeItem(pajaroItem);
        delete pajaroItem;
    }

    // Crear pájaro con tamaño adecuado (ajustar escala si es necesario)
    pajaroItem = new QGraphicsPixmapItem(framesPajaro[0]);
    pajaroItem->setScale(0.7);  // Ajustar según necesidad
    pajaroItem->setZValue(-0.5);
    escena->addItem(pajaroItem);

    // Posición inicial más variable
    pajaroItem->setPos(1280, QRandomGenerator::global()->bounded(5, 95));

    // Configurar timer de animación más rápido
    if (!timerAnimacionPajaro) {
        timerAnimacionPajaro = new QTimer(this);
        connect(timerAnimacionPajaro, &QTimer::timeout, this, &Juego::animarPajaro);
    }
    timerAnimacionPajaro->start(40);  // Más rápido (12-13 FPS)
}

void Juego::animarPajaro()
{
    if (!pajaroItem || framesPajaro.empty()) return;

    // Avanzar animación
    frameActualPajaro = (frameActualPajaro + 1) % framesPajaro.size();
    pajaroItem->setPixmap(framesPajaro[frameActualPajaro]);

    // Mover más rápido (8px por frame en lugar de 5)
    pajaroItem->moveBy(-8, 0);

    // Reposicionar cuando esté completamente fuera
    if (pajaroItem->x() + pajaroItem->boundingRect().width() * pajaroItem->scale() < 0) {
        pajaroItem->setPos(1280, QRandomGenerator::global()->bounded(5, 95));

        // 80% de probabilidad de mostrar otro pájaro
        pajaroItem->setVisible(QRandomGenerator::global()->bounded(100) < 70);

        // Variar velocidad ocasionalmente
        if (QRandomGenerator::global()->bounded(100) < 30) {
            timerAnimacionPajaro->start(30); // Más rápido
        } else {
            timerAnimacionPajaro->start(40); // Velocidad normal
        }
    }
}

void Juego::actualizarDecoracion()
{
    // Mover palmeras existentes
    for (auto palmera : decoracionPalmeras) {
        palmera->moveBy(-velocidadScroll, 0);

        if (palmera->x() + palmera->boundingRect().width() * palmera->scale() < 0) {
            // Guardamos la posición Y actual antes de reposicionar
            qreal currentY = palmera->y();

            // AUMENTAR EL RANGO ALEATORIO PARA QUE APAREZCAN MENOS FRECUENTEMENTE
            if (QRandomGenerator::global()->bounded(100) < 30) { // 30% de probabilidad
                qreal newX = 1280 + QRandomGenerator::global()->bounded(3000, 6000);
                palmera->setPos(newX, currentY);
            } else {
                // Si no cumple la probabilidad, colocar muy lejos para que tarde en aparecer
                palmera->setPos(10000, currentY);
            }
        }
    }

    // OPCIONAL: Generar palmeras nuevas con baja frecuencia
    static int contador = 0;
    if (contador++ > 100 && decoracionPalmeras.size() < 8) { // Cada 100 frames
        contador = 0;
        if (QRandomGenerator::global()->bounded(100) < 20) { // 20% de probabilidad
            // Crear nueva palmera
            QGraphicsPixmapItem* palmera = new QGraphicsPixmapItem(spritesPalmeras[0]);
            palmera->setScale(2.0);
            palmera->setPos(1280 + QRandomGenerator::global()->bounded(2000),
                            450 + QRandomGenerator::global()->bounded(150));
            palmera->setZValue(-1);
            escena->addItem(palmera);
            decoracionPalmeras.append(palmera);
        }
    }
}

void Juego::mostrarGameOver() {
    detenerTodo();

    // Mostrar mensaje de Game Over
    QGraphicsTextItem* gameOverText = new QGraphicsTextItem("GAME OVER");
    gameOverText->setDefaultTextColor(Qt::red);
    gameOverText->setFont(QFont(dragonBallFont, 72, QFont::Bold));

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);
    shadow->setColor(Qt::black);
    shadow->setOffset(5, 5);
    gameOverText->setGraphicsEffect(shadow);

    gameOverText->setPos(width()/2 - gameOverText->boundingRect().width()/2,
                         height()/2 - 100);
    gameOverText->setZValue(1000);
    escena->addItem(gameOverText);

    // Programar la emisión de la señal después de un breve retraso
    timerGameOver->start(3000); // 3 segundos para ver el mensaje
}

void Juego::mostrarMenuGameOver() {
    emit gameOver(); // Ahora emitimos la señal después del retraso
}

void Juego::reiniciarJuego() {
    // Limpiar todos los elementos del juego
    detenerTodo();

    // Limpiar enemigos
    for (auto enemigo : enemigos) {
        enemigo->eliminarProyectiles();
        escena->removeItem(enemigo);
        delete enemigo;
    }
    enemigos.clear();

    // Limpiar cápsulas
    for (auto capsula : capsulas) {
        escena->removeItem(capsula);
        delete capsula;
    }
    capsulas.clear();

    // Limpiar plataformas
    for (auto plataforma : plataformas) {
        escena->removeItem(plataforma);
        delete plataforma;
    }
    plataformas.clear();

    // Limpiar obstáculos
    for (auto obstaculo : obstaculos) {
        escena->removeItem(obstaculo);
        delete obstaculo;
    }
    obstaculos.clear();

    // Limpiar cuerdas
    for (auto& cuerda : cuerdas) {
        escena->removeItem(cuerda.cuerdaItem);
        if (cuerda.gokuSprite) escena->removeItem(cuerda.gokuSprite);
        delete cuerda.cuerdaItem;
        delete cuerda.gokuSprite;
    }
    cuerdas.clear();

    // Reiniciar tronco
    if (troncoActual.sprite) {
        escena->removeItem(troncoActual.sprite);
        delete troncoActual.sprite;
        troncoActual.sprite = nullptr;
    }

    // Reiniciar variables de estado
    velocidadScroll = 3.0;
    soldadosEliminados = 0;
    pausado = false;
    gokuEnCuerda = false;

    // Reiniciar Goku
    goku->setPos(0, 450);
    goku->setVisible(true);

    // Reiniciar contador de soldados (usando el mismo formato que en el constructor)
    contadorSoldados->setHtml(
        "<div style='color: #ffcc00; font-family: \"" + dragonBallFont + "\"; font-size: 18px; "
                                                                         "font-weight: bold; text-shadow: 3px 3px 5px #000000; letter-spacing: 2px;'>"
                                                                         "ENEMIGOS: <span style='color: #ffffff;'>0</span>/" + QString::number(OBJETIVO_SOLDADOS) +
        "</div>"
        );

    // Reiniciar barra de energía
    actualizarBarraEnergia();

    // Volver a generar elementos iniciales
    generarPlataforma();
    QTimer::singleShot(7000, this, &Juego::generarTroncoUnico);

    // Iniciar juego nuevamente
    iniciar();
}

Juego::~Juego() {
    // Limpiar decoración
    for (auto palmera : decoracionPalmeras) {
        escena->removeItem(palmera);
        delete palmera;
    }
    decoracionPalmeras.clear();

    if (pajaroItem) {
        escena->removeItem(pajaroItem);
        delete pajaroItem;
    }

    if (timerAnimacionPajaro) {
        timerAnimacionPajaro->stop();
        delete timerAnimacionPajaro;
    }

    // ... resto de la limpieza ...
}
