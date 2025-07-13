#include "juego2.h"
#include <QGraphicsPixmapItem>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QTimeLine>
#include <QGraphicsColorizeEffect>
#include <QFontDatabase>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QDialog>
#include <QMovie>
#include <QVBoxLayout>
#include <QLabel>

/**
 * @brief Constructor del nivel 2 del juego.
 *
 * Inicializa la escena, configura el fondo, la música, las fuentes, el mapa,
 * los elementos gráficos como barra de energía, llaves, botones de pausa
 * y conecta los temporizadores principales.
 */
Juego2::Juego2(QWidget* parent) : QGraphicsView(parent) {
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 1280, 680);
    setScene(escena);
    setFixedSize(1280, 680);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    musicaNivel2 = new QMediaPlayer(this);
    audioNivel2 = new QAudioOutput(this);
    musicaNivel2->setAudioOutput(audioNivel2);
    musicaNivel2->setSource(QUrl("qrc:/sounds/Sounds/musica_nivel2.mp3"));
    audioNivel2->setVolume(1.0);
    musicaNivel2->setLoops(QMediaPlayer::Infinite);
    musicaNivel2->play();

    // Cargar fuente Dragon Ball
    int fontId = QFontDatabase::addApplicationFont(":/fondos/Pictures/db_font.ttf");
    if (fontId == -1) {
        qDebug() << "Error al cargar la fuente Dragon Ball";
        dragonBallFont = "Arial";
    } else {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            dragonBallFont = fontFamilies.first();
        } else {
            qDebug() << "No se encontraron familias de fuentes";
            dragonBallFont = "Arial";
        }
    }

    // Fondo de nivel 2
    QPixmap fondo(":/fondos/Pictures/fondo_nivel2.jpg");
    fondo = fondo.scaled(1280, 680, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QGraphicsPixmapItem* fondoItem = new QGraphicsPixmapItem(fondo);
    fondoItem->setZValue(-1);  // Enviar al fondo
    escena->addItem(fondoItem);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    cargarMapa("");

    // Barra de energía visual estilo nivel 1
    barraEnergia = new QGraphicsRectItem(25, 25, 0, 20);
    QLinearGradient gradEnergia(0, 0, 200, 0);
    gradEnergia.setColorAt(0, QColor(255, 255, 0));    // amarillo
    gradEnergia.setColorAt(0.5, QColor(255, 165, 0));  // naranja
    gradEnergia.setColorAt(1, QColor(255, 0, 0));      // rojo
    barraEnergia->setBrush(QBrush(gradEnergia));
    barraEnergia->setPen(QPen(QColor(255, 215, 0), 2));  // borde dorado
    barraEnergia->setZValue(11);
    barraEnergia->setPos(1050, 0);
    escena->addItem(barraEnergia);

    // Texto "ENERGIA" con fuente Dragon Ball
    QGraphicsTextItem* energiaText = new QGraphicsTextItem("ENERGIA");
    energiaText->setDefaultTextColor(QColor(255, 215, 0));
    energiaText->setFont(QFont(dragonBallFont, 16, QFont::Bold));
    energiaText->setPos(1070, 0);
    energiaText->setZValue(12);
    escena->addItem(energiaText);

    // Texto de llaves con fuente Dragon Ball
    textoLlaves = new QGraphicsTextItem();
    textoLlaves->setHtml(
        "<div style='color: #ffcc00; font-family: \"" + dragonBallFont + "\"; font-size: 18px; "
                                                                         "font-weight: bold; text-shadow: 3px 3px 5px #000000; letter-spacing: 2px;'>"
                                                                         "LLAVES: <span style='color: #ffffff;'>0</span>/4</div>"
        );
    textoLlaves->setZValue(12);
    textoLlaves->setPos(1070, 40);  // Evita solaparse con barra energía
    escena->addItem(textoLlaves);

    // Botón de pausa
    QPushButton* botonPausa = new QPushButton(this);
    botonPausa->setGeometry(0, 0, 120, 50);
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
    connect(botonPausa, &QPushButton::clicked, this, &Juego2::togglePausa);

    timerActualizar = new QTimer(this);
    connect(timerActualizar, &QTimer::timeout, this, &Juego2::actualizar);
    timerActualizar->start(100);  // cada 100 ms

    timerGameOver = new QTimer(this);
    timerGameOver->setSingleShot(true);
    connect(timerGameOver, &QTimer::timeout, this, &Juego2::mostrarMenuGameOver);
}

/**
 * @brief Ciclo principal de actualización del nivel 2.
 *
 * Revisa colisiones entre Goku y cápsulas, llaves, enemigos y Bulma.
 * Gestiona el avance del juego, condiciones de victoria y derrota.
 */
void Juego2::actualizar() {
    if (enPausa) return;
    // 1. Revisar colisiones con cápsulas
    for (int i = 0; i < capsulas.size(); ++i) {
        QGraphicsPixmapItem* capsula = capsulas[i];
        if (goku->collidesWithItem(capsula)) {
            if (goku->obtenerEnergia() < 100) {
                escena->removeItem(capsula);
                delete capsula;
                capsulas.remove(i);
                goku->aumentarEnergia(100);  // una sola cápsula llena la barra
                actualizarBarraEnergia();    // si ya tienes la barra visual implementada
                break;
            }
        }
    }

    // 2. Revisar colisiones con llaves
    for (int i = 0; i < llaves.size(); ++i) {
        QGraphicsPixmapItem* llave = llaves[i];
        if (goku->collidesWithItem(llave)) {
            escena->removeItem(llave);
            delete llave;
            llaves.remove(i);
            ++llavesRecogidas;
            actualizarContadorLlaves();
            if (llavesRecogidas == TOTAL_LLAVES) {
                abrirPuerta();
            }
            break;
        }
    }

    // 3. Colisión con soldados
    for (Enemigo* enemigo : enemigos) {
        if (goku->collidesWithItem(enemigo)) {
            qDebug() << "Goku fue tocado por un soldado. GAME OVER.";
            mostrarGameOver();
            return;
        }
    }

    // 4. Verificar victoria al tocar a Bulma
    if (llavesRecogidas == TOTAL_LLAVES && bulma && !m_nivelCompletado) {
        if (goku->collidesWithItem(bulma)) {
            m_nivelCompletado = true;
            enPausa = true;
            detenerTodo();
            if (musicaNivel2 && musicaNivel2->playbackState() == QMediaPlayer::PlayingState) musicaNivel2->stop();

            QMediaPlayer* victoria = new QMediaPlayer(this);
            QAudioOutput* audioVictoria = new QAudioOutput(this);
            victoria->setAudioOutput(audioVictoria);
            victoria->setSource(QUrl("qrc:/sounds/Sounds/victoria.wav"));
            audioVictoria->setVolume(100);
            victoria->play();

            // Mostrar mensaje personalizado de victoria
            QDialog* victoriaDialog = new QDialog(this);
            victoriaDialog->setWindowTitle("¡Misión cumplida!");
            victoriaDialog->setStyleSheet(R"(
    QDialog {
        background-color: #2c3e50;
    }
    QLabel {
        color: white;
        font: bold 16px;
    }
    QPushButton {
        background-color: #27ae60;
        color: white;
        border-radius: 5px;
        padding: 5px 10px;
        min-width: 80px;
    }
    QPushButton:hover {
        background-color: #2ecc71;
    }
)");

            QVBoxLayout* layout = new QVBoxLayout(victoriaDialog);

            // Imagen GIF
            QLabel* gifLabel = new QLabel;
            QMovie* movie = new QMovie(":/fondos/Pictures/goku-zzz.gif");
            gifLabel->setMovie(movie);
            movie->start();

            // Mensaje
            QLabel* mensaje = new QLabel("¡Has rescatado a Bulma!<br><br>Goku, una vez más, salva el día.<br><br>Bulma te debe una... <i>¡otra vez!</i>");
            mensaje->setTextFormat(Qt::RichText);
            mensaje->setAlignment(Qt::AlignCenter);

            // Botón cerrar
            QPushButton* btnCerrar = new QPushButton("Cerrar");
            connect(btnCerrar, &QPushButton::clicked, victoriaDialog, &QDialog::accept);

            // Añadir al layout
            layout->addWidget(gifLabel, 0, Qt::AlignCenter);
            layout->addWidget(mensaje);
            layout->addWidget(btnCerrar, 0, Qt::AlignCenter);

            victoriaDialog->exec();
            close();
        }
    }
}

/**
 * @brief Carga el mapa del nivel 2 desde una matriz de caracteres codificada.
 *
 * Interpreta los siguientes símbolos:
 * '#' = muro, '.' = camino, 'G' = Goku, 'E' = enemigo, 'K' = llave,
 * 'C' = cápsula, 'B' = Bulma, 'D' = puerta.
 *
 * @param archivo Ruta del archivo de mapa (no utilizado actualmente).
 */
void Juego2::cargarMapa(const QString&) {
    QStringList mapa = {
        "###########################",
        "#.....K......##.........K.#",
        "#.####.#####.##.#####.#####",
        "#G####.#####.##.#####E#####",
        "#.####.#####.##.#####.#####",
        "#.................C.......#",
        "#.####.##.########.##.###.#",
        "#.####.##...######.##.###.#",
        "#.........#..##....##....E#",
        "######.##.###########.#####",
        "#    #.##.#...B....##.#   #",
        "#    C ##.#.........#.    #",
        "#    #.##.########..#.#K  #",
        "######.##.########D##.#####",
        "#E...........##...........#",
        "#.####.#####.##.#####.#####",
        "#.####.#####E##.#####.#####",
        "#....#.#.................##",
        "###.##.##.########.##.#..##",
        "###..#.##.#########..E##.##",
        "#....#E#K........####.....#",
        "#.##############.########.#",
        "#.......C.................#",
        "###########################"
    };

    const int tileWidth = 1280 / 31;
    const int tileHeight = 680 / 23;

    // Carga de imágenes (una sola vez)
    QPixmap imgMuro(":/fondos/Pictures/textura_roca.png");
    QPixmap imgLlave(":/sprites/Pictures/llave.png");
    QPixmap imgCapsula(":/sprites/Pictures/capsula.png");
    QPixmap imgPuerta(":/sprites/Pictures/puerta.png");
    QPixmap imgBulma(":/sprites/Pictures/bulma.png");

    // Escalar imágenes una vez
    QPixmap scaledMuro = imgMuro.scaled(tileWidth, tileHeight);
    QPixmap scaledLlave = imgLlave.scaled(tileWidth, tileHeight);
    QPixmap scaledCapsula = imgCapsula.scaled(tileWidth, tileHeight);
    QPixmap scaledPuerta = imgPuerta.scaled(tileWidth, tileHeight);

    // Frame 1 de Bulma (índice 1 de 4 frames)
    int w = imgBulma.width() / 4;
    int h = imgBulma.height();
    QPixmap frameBulma = imgBulma.copy(w, 0, w, h).scaled(50, 50, Qt::KeepAspectRatio);

    for (int fila = 0; fila < mapa.size(); ++fila) {
        QString linea = mapa[fila];
        for (int col = 0; col < linea.length(); ++col) {
            QChar simbolo = linea[col];
            int x = col * tileWidth;
            int y = fila * tileHeight;

            switch (simbolo.toLatin1()) {
            case '#': {
                QGraphicsPixmapItem* muro = new QGraphicsPixmapItem(scaledMuro);
                muro->setPos(x, y);
                muro->setZValue(0);
                muro->setData(0, "muro");
                escena->addItem(muro);
                break;
            }
            case '.':
                break; // camino, el fondo ya lo cubre
            case 'G': {
                goku = new Goku(x, y, tileWidth, tileHeight);
                connect(goku, &Goku::actualizarBarraEnergiaSignal, this, &Juego2::actualizarBarraEnergia);
                connect(goku, &Personaje::gokuDerrotado, this, &Juego2::mostrarGameOver);
                goku->cargarAnimacionesNivel2(); // Sprite cenital
                goku->setZValue(2);
                escena->addItem(goku);
                goku->setFlag(QGraphicsItem::ItemIsFocusable);
                goku->setFocus();
                break;
            }
            case 'E': {
                Enemigo* enemigo = new Enemigo(x, y, tileWidth, tileHeight, goku, true); // true → esNivel2
                enemigo->cargarAnimacionesNivel2(tileWidth + 10, tileHeight + 10);
                enemigo->setZValue(2);
                enemigo->setData(0, "enemigo");
                enemigos.append(enemigo);
                escena->addItem(enemigo);
                break;
            }
            case 'K': {
                QGraphicsPixmapItem* llave = new QGraphicsPixmapItem(scaledLlave);
                llave->setPos(x, y);
                llave->setZValue(1);
                llaves.append(llave);
                escena->addItem(llave);
                break;
            }
            case 'C': {
                QGraphicsPixmapItem* capsula = new QGraphicsPixmapItem(scaledCapsula);
                capsula->setPos(x, y);
                capsula->setZValue(1);
                capsulas.append(capsula);
                escena->addItem(capsula);
                break;
            }
            case 'B': {
                bulma = new QGraphicsPixmapItem(frameBulma);
                bulma->setPos(x, y);
                escena->addItem(bulma);
                break;
            }
            case 'D': {
                QGraphicsPixmapItem* puertaItem = new QGraphicsPixmapItem(scaledPuerta);
                puertaItem->setPos(x, y);
                puertaItem->setData(0, "puerta");
                puerta = puertaItem;
                escena->addItem(puerta);
                break;
            }
            default:
                break;
            }
        }
    }
}

/**
 * @brief Actualiza gráficamente la barra de energía de Goku.
 *
 * Anima el crecimiento de la barra y muestra un efecto visual si la energía está completa.
 */
void Juego2::actualizarBarraEnergia() {
    if (!goku || !barraEnergia) return;

    float porcentaje = static_cast<float>(goku->obtenerEnergia()) / 100.0f;
    QRectF targetRect(25, 25, porcentaje * 200, 20);  // ancho dinámico

    QTimeLine* anim = new QTimeLine(200, this);  // 200ms de animación
    anim->setFrameRange(0, 100);

    QRectF rectInicio = barraEnergia->rect();  // valor actual

    connect(anim, &QTimeLine::frameChanged, [=](int) {
        qreal progress = anim->currentValue();
        QRectF interpolado(
            rectInicio.x(),
            rectInicio.y(),
            rectInicio.width() + (targetRect.width() - rectInicio.width()) * progress,
            rectInicio.height()
            );
        barraEnergia->setRect(interpolado);
    });

    connect(anim, &QTimeLine::finished, anim, &QTimeLine::deleteLater);
    anim->start();

    // Efecto brillante cuando está llena
    if (goku->puedeDisparar()) {
        QGraphicsColorizeEffect* efecto = new QGraphicsColorizeEffect(this);
        efecto->setColor(QColor(255, 255, 255, 150));
        barraEnergia->setGraphicsEffect(efecto);
        QTimer::singleShot(500, this, [=]() {
            barraEnergia->setGraphicsEffect(nullptr);
        });
    }
}

/**
 * @brief Actualiza el contador visual de llaves recolectadas.
 */
void Juego2::actualizarContadorLlaves() {
    if (textoLlaves) {
        textoLlaves->setHtml(
            "<div style='color: #ffcc00; font-family: \"" + dragonBallFont + "\"; font-size: 18px; "
                                                                             "font-weight: bold; text-shadow: 3px 3px 5px #000000; letter-spacing: 2px;'>"
                                                                             "LLAVES: <span style='color: #ffffff;'>" + QString::number(llavesRecogidas) + "</span>/" +
            QString::number(TOTAL_LLAVES) +
            "</div>"
            );
    }
}

/**
 * @brief Abre la puerta en el mapa cuando se recolectan todas las llaves.
 *
 * Muestra un mensaje animado "PUERTA ABIERTA" en pantalla.
 */
void Juego2::abrirPuerta() {
    if (puerta) {
        escena->removeItem(puerta);
        delete puerta;
        puerta = nullptr;
    }

    // Mostrar mensaje animado "PUERTA ABIERTA"
    QGraphicsTextItem* mensajePuerta = new QGraphicsTextItem("PUERTA ABIERTA");
    mensajePuerta->setDefaultTextColor(QColor(255, 215, 0));  // color dorado
    mensajePuerta->setFont(QFont(dragonBallFont, 32, QFont::Bold));

    mensajePuerta->setZValue(1001);
    mensajePuerta->setPos(width()/2 - mensajePuerta->boundingRect().width()/2, height()/2 - 60);

    // Sombra
    QGraphicsDropShadowEffect* sombra = new QGraphicsDropShadowEffect();
    sombra->setBlurRadius(8);
    sombra->setColor(Qt::black);
    sombra->setOffset(4, 4);
    mensajePuerta->setGraphicsEffect(sombra);

    escena->addItem(mensajePuerta);

    // Eliminar mensaje después de 2.5 segundos
    QTimer::singleShot(2500, this, [=]() {
        escena->removeItem(mensajePuerta);
        delete mensajePuerta;
    });
}

/**
 * @brief Elimina un enemigo de la escena y limpia sus proyectiles.
 *
 * @param enemigo Puntero al enemigo a eliminar.
 */
void Juego2::eliminarEnemigo(Enemigo* enemigo) {
    if (!enemigo) return;

    enemigo->eliminarProyectiles();
    enemigos.removeOne(enemigo);
    escena->removeItem(enemigo);
    delete enemigo;
}

/**
 * @brief Muestra la pantalla de Game Over y reproduce sonido de derrota.
 *
 * También emite la señal `gameOver()` después de 3 segundos.
 */
void Juego2::mostrarGameOver() {
    detenerTodo();
    if (musicaNivel2 && musicaNivel2->playbackState() == QMediaPlayer::PlayingState) musicaNivel2->stop();

    QMediaPlayer* derrota = new QMediaPlayer;
    QAudioOutput* salidaDerrota = new QAudioOutput;
    derrota->setAudioOutput(salidaDerrota);
    derrota->setSource(QUrl("qrc:/sounds/Sounds/derrota.mp3"));
    salidaDerrota->setVolume(80);
    derrota->play();
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

/**
 * @brief Emite la señal de Game Over tras una pausa.
 */
void Juego2::mostrarMenuGameOver() {
    emit gameOver(); // Ahora emitimos la señal después del retraso
}

/**
 * @brief Método vacío que puede usarse para inicializar el juego desde MainWindow.
 */
void Juego2::iniciar() {
    // Comienza el juego (activar timers, etc.)
    // Por ahora vacío, pero lo puedes llenar si es necesario.
}

/**
 * @brief Agrega un ítem a la escena si esta existe.
 *
 * @param item Puntero al QGraphicsItem a agregar.
 */
void Juego2::agregarItemEscena(QGraphicsItem* item) {
    if (escena) escena->addItem(item);
}

/**
 * @brief Remueve un ítem de la escena si existe.
 *
 * @param item Puntero al QGraphicsItem a remover.
 */
void Juego2::removerItemEscena(QGraphicsItem* item) {
    if (escena && item) escena->removeItem(item);
}

/**
 * @brief Alterna entre pausa y reanudación del juego.
 *
 * Muestra un overlay visual con el texto "PAUSA" y botones para continuar o salir al menú.
 * Oculta el overlay y reanuda la lógica cuando se quita la pausa.
 */
void Juego2::togglePausa() {
    pausado = !pausado;

    if (pausado) {
        detenerTodo();  // Detener enemigos, Goku y timer

        // Fondo semitransparente
        QGraphicsRectItem* overlay = new QGraphicsRectItem(0, 0, width(), height());
        overlay->setBrush(QColor(0, 0, 0, 180));
        overlay->setZValue(999);
        overlay->setData(0, "pausa_overlay");
        escena->addItem(overlay);

        // Texto "PAUSA"
        QGraphicsTextItem* pausaText = new QGraphicsTextItem("PAUSE");
        pausaText->setDefaultTextColor(QColor(255, 215, 0));
        pausaText->setFont(QFont(dragonBallFont, 58, QFont::Bold));
        pausaText->setZValue(1001);
        pausaText->setPos(width()/2 - pausaText->boundingRect().width()/2, height()/2 - 120);
        pausaText->setData(0, "pausa_overlay");
        escena->addItem(pausaText);

        if (!btnContinuar) {
            configurarBotonesPausa();
        } else {
            proxyContinuar->show();
            proxySalir->show();
        }

    } else {
        // Quitar overlay
        for (QGraphicsItem* item : escena->items()) {
            if (item->data(0).toString() == "pausa_overlay") {
                escena->removeItem(item);
                delete item;
            }
        }

        if (proxyContinuar) proxyContinuar->hide();
        if (proxySalir) proxySalir->hide();

        reanudarTodo();  // Reanudar lógica
        if (goku) goku->setFocus();
    }
}

/**
 * @brief Configura los botones interactivos del menú de pausa.
 *
 * Incluye los botones "CONTINUAR" y "SALIR AL MENÚ".
 */
void Juego2::configurarBotonesPausa() {
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

    proxyContinuar = escena->addWidget(btnContinuar);
    proxyContinuar->setPos(width()/2 - btnContinuar->width()/2, height()/2 + 30);
    proxyContinuar->setZValue(1002);

    proxySalir = escena->addWidget(btnSalir);
    proxySalir->setPos(width()/2 - btnSalir->width()/2, height()/2 + 110);
    proxySalir->setZValue(1002);

    // Conexión botón CONTINUAR
    connect(btnContinuar, &QPushButton::clicked, this, &Juego2::togglePausa);

    // Conexión botón SALIR AL MENÚ con pausa previa
    connect(btnSalir, &QPushButton::clicked, this, [this]() {
        if (pausado) togglePausa();  // Quitar el overlay de pausa
        emit salirAlMenu();          // Emitir señal al MainWindow
        detenerTodo();
        if (musicaNivel2 && musicaNivel2->playbackState() == QMediaPlayer::PlayingState) musicaNivel2->stop();
    });
}

/**
 * @brief Detiene toda la lógica del juego: timers, enemigos y controles de Goku.
 */
void Juego2::detenerTodo() {
    if (goku) goku->setEnabled(false);  // Desactiva teclado

    for (Enemigo* enemigo : enemigos) {
        if (enemigo) enemigo->detener();
    }

    if (timerActualizar) timerActualizar->stop();  // Pausar colisiones
}

/**
 * @brief Reanuda la lógica detenida del juego y devuelve el foco a Goku.
 */
void Juego2::reanudarTodo() {
    if (goku) {
        goku->setEnabled(true);  // Reactiva teclado
        goku->setFocus();
    }

    for (Enemigo* enemigo : enemigos) {
        if (enemigo) enemigo->reanudar();
    }

    if (timerActualizar) timerActualizar->start();
}
