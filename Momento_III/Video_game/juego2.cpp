#include "juego2.h"
#include <QGraphicsPixmapItem>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QTimeLine>
#include <QGraphicsColorizeEffect>
#include <QFontDatabase>

Juego2::Juego2(QWidget* parent) : QGraphicsView(parent) {
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 1280, 680);
    setScene(escena);
    setFixedSize(1280, 680);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 🎯 Cargar fuente Dragon Ball
    QString dragonBallFont;
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

    // 🔶 Fondo de nivel 2
    QPixmap fondo(":/fondos/Pictures/fondo_nivel2.jpg");
    fondo = fondo.scaled(1280, 680, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QGraphicsPixmapItem* fondoItem = new QGraphicsPixmapItem(fondo);
    fondoItem->setZValue(-1);  // Enviar al fondo
    escena->addItem(fondoItem);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    cargarMapa("");

    // 🔋 Barra de energía visual estilo nivel 1
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

    // 🟨 Texto "ENERGIA" con fuente Dragon Ball
    QGraphicsTextItem* energiaText = new QGraphicsTextItem("ENERGIA");
    energiaText->setDefaultTextColor(QColor(255, 215, 0));
    energiaText->setFont(QFont(dragonBallFont, 16, QFont::Bold));
    energiaText->setPos(1070, 0);
    energiaText->setZValue(12);
    escena->addItem(energiaText);

    // 🔑 Texto de llaves con fuente Dragon Ball
    textoLlaves = new QGraphicsTextItem();
    textoLlaves->setHtml(
        "<div style='color: #ffcc00; font-family: \"" + dragonBallFont + "\"; font-size: 18px; "
                                                                         "font-weight: bold; text-shadow: 3px 3px 5px #000000; letter-spacing: 2px;'>"
                                                                         "LLAVES: <span style='color: #ffffff;'>0</span>/4</div>"
        );
    textoLlaves->setZValue(12);
    textoLlaves->setPos(1070, 40);  // 📍 Evita solaparse con barra energía
    escena->addItem(textoLlaves);

    QTimer* timerActualizar = new QTimer(this);
    connect(timerActualizar, &QTimer::timeout, this, &Juego2::actualizar);
    timerActualizar->start(100);  // cada 100 ms
}

void Juego2::actualizar() {
    // 1. Revisar colisiones con cápsulas
    for (int i = 0; i < capsulas.size(); ++i) {
        QGraphicsPixmapItem* capsula = capsulas[i];
        if (goku->collidesWithItem(capsula)) {
            if (goku->obtenerEnergia() < 100) {
                escena->removeItem(capsula);
                delete capsula;
                capsulas.remove(i);
                goku->aumentarEnergia(100);  // ✅ una sola cápsula llena la barra
                actualizarBarraEnergia();    // 🔁 si ya tienes la barra visual implementada
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
            // 🔻 Aquí puedes reiniciar el nivel o mostrar pantalla de derrota
            // ejemplo futuro: emit gameOver();
            return;
        }
    }

    // 4. Verificar victoria al tocar a Bulma
    if (llavesRecogidas == TOTAL_LLAVES && bulma && !nivelCompletado) {
        if (goku->collidesWithItem(bulma)) {
            nivelCompletado = true;

            // Mostrar mensaje de victoria
            QMessageBox::information(this, "¡Victoria!", "¡Has rescatado a Bulma!\n¡Felicidades!");

            // Aquí podrías cerrar el juego, regresar al menú, etc.
            // Por ahora cerramos la ventana:
            close();
        }
    }
}

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

    // 🔁 Carga de imágenes (una sola vez)
    QPixmap imgMuro(":/fondos/Pictures/textura_roca.png");
    QPixmap imgLlave(":/sprites/Pictures/llave.png");
    QPixmap imgCapsula(":/sprites/Pictures/capsula.png");
    QPixmap imgPuerta(":/sprites/Pictures/puerta.png");
    QPixmap imgBulma(":/sprites/Pictures/bulma.png");

    // 🔁 Escalar imágenes una vez
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

void Juego2::actualizarBarraEnergia() {
    if (!goku || !barraEnergia) return;

    float porcentaje = static_cast<float>(goku->obtenerEnergia()) / 100.0f;
    QRectF targetRect(25, 25, porcentaje * 200, 20);  // ancho dinámico

    QTimeLine* anim = new QTimeLine(200, this);  // 200ms de animación
    anim->setFrameRange(0, 100);

    QRectF rectInicio = barraEnergia->rect();  // valor actual

    connect(anim, &QTimeLine::frameChanged, [=](int frame) {
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

void Juego2::actualizarContadorLlaves() {
    if (textoLlaves) {
        textoLlaves->setPlainText(QString("Llaves: %1/%2").arg(llavesRecogidas).arg(TOTAL_LLAVES));
    }
}

void Juego2::abrirPuerta() {
    if (puerta) {
        escena->removeItem(puerta);
        delete puerta;
        puerta = nullptr;
    }
}

void Juego2::eliminarEnemigo(Enemigo* enemigo) {
    if (!enemigo) return;

    enemigo->eliminarProyectiles();
    enemigos.removeOne(enemigo);
    escena->removeItem(enemigo);
    delete enemigo;
}
