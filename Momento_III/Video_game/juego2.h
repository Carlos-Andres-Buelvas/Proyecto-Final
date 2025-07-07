#ifndef JUEGO2_H
#define JUEGO2_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QSet>
#include "goku.h"
#include "enemigo.h"

class Juego2 : public QGraphicsView {
    Q_OBJECT
public:
    explicit Juego2(QWidget* parent = nullptr);
    void cargarMapa(const QString& archivo);
    Goku* goku = nullptr;

    void actualizarBarraEnergia();
    void eliminarEnemigo(Enemigo* enemigo);
private slots:
    void actualizar();

private:
    QGraphicsScene* escena;
    QVector<Enemigo*> enemigos;
    QVector<QGraphicsPixmapItem*> llaves;
    QVector<QGraphicsPixmapItem*> capsulas;
    QGraphicsPixmapItem* bulma;
    QGraphicsPixmapItem* puerta;

    int llavesRecogidas = 0;
    const int TOTAL_LLAVES = 4;

    void verificarVictoria();
    void abrirPuerta();


    //QGraphicsRectItem* fondoEnergia = nullptr;
    QGraphicsRectItem* barraEnergia = nullptr;
    QGraphicsTextItem* textoLlaves = nullptr;

    void actualizarContadorLlaves();

    bool nivelCompletado = false;
};

#endif // JUEGO2_H
