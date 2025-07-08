#ifndef JUEGO2_H
#define JUEGO2_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QSet>
#include <QPushButton>
#include <QGraphicsProxyWidget>
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
    void mostrarGameOver();
    void iniciar();  // Llamado después del título
    void agregarItemEscena(QGraphicsItem* item);
    void removerItemEscena(QGraphicsItem* item);
    void mostrarTituloNivel();
    void reanudarTodo();
    void detenerTodo();

signals:
    void gameOver();
    void salirAlMenu();

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
    bool gameOverMostrado = false;
    bool enPausa = false;

    QTimer* timerActualizar = nullptr;
    bool pausado = false;

    QPushButton* btnContinuar = nullptr;
    QPushButton* btnSalir = nullptr;
    QGraphicsProxyWidget* proxyContinuar = nullptr;
    QGraphicsProxyWidget* proxySalir = nullptr;

    void togglePausa();
    void configurarBotonesPausa();

    QString dragonBallFont;
};

#endif // JUEGO2_H
