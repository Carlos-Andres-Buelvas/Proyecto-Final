#include "juego2.h"
#include <QGraphicsPixmapItem>
#include <QFile>
#include <QTextStream>
#include <QDebug>

Juego2::Juego2(QWidget* parent) : QGraphicsView(parent) {
    escena = new QGraphicsScene(this);
    escena->setSceneRect(0, 0, 1280, 680);
    setScene(escena);
    setFixedSize(1280, 680);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 🔶 Fondo de nivel 2
    QPixmap fondo(":/fondos/Pictures/fondo_nivel2.jpg");
    fondo = fondo.scaled(1280, 680, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QGraphicsPixmapItem* fondoItem = new QGraphicsPixmapItem(fondo);
    fondoItem->setZValue(-1);  // Enviar al fondo
    escena->addItem(fondoItem);

    cargarMapa("");

}

void Juego2::actualizar() {
    // Implementación temporal para compilar
    // Aquí más adelante irá la lógica de colisiones, patrullaje, etc.
}

void Juego2::cargarMapa(const QString&) {
    QStringList mapa = {
        "###########################",
        "#.....K......##.........K.#",
        "#.####.#####.##.#####.#####",
        "#G####.#####.##.#####E#####",
        "#.####.#####.##.#####.#####",
        "#.................C.......#",
        "#.####.##.########.##.#####",
        "#.####.##.########.##.#####",
        "#......##....##....##....E#",
        "######.##############.#####",
        "#    #.####...B....##.#   #",
        "#    C ####.........#.    #",
        "#    #.###########..#.#K  #",
        "######.##.########D##.#####",
        "#E...........##...........#",
        "#.####.#####.##.#####.#####",
        "#.####.#####E##.#####.#####",
        "#....#.#.................##",
        "###.##.##.########.##.#..##",
        "###.##.##.#########..E##.##",
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
                goku->cargarAnimacionesNivel2(); // Sprite cenital
                goku->setZValue(2);
                escena->addItem(goku);
                goku->setFocus();
                break;
            }
            case 'E': {
                Enemigo* enemigo = new Enemigo(x, y, 1000, 2000, goku);
                enemigo->cargarAnimacionesNivel2(tileWidth + 10, tileHeight + 10);
                enemigo->setZValue(2);
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
