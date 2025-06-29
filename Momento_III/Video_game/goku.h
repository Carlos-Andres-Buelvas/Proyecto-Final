#ifndef GOKU_H
#define GOKU_H

#include "personaje.h"
#include <QVector>
#include <QPixmap>
#include <QGraphicsScene>

class Enemigo;

class Goku : public Personaje {
    Q_OBJECT

public:
    Goku(float x, float y, float ancho, float alto);

    void mover() override;
    void disparar(QGraphicsScene* escena) override;
    void saltar();
    void animarCorrer();
    void animarCaida();
    void animarDisparo();
    void animarCuerda();
    void acelerarCaida(); // tecla S
    bool teclaWSostenida = false;
    bool cayendoLento = false;
    bool estaDisparando() const;

    bool puedeDisparar() const { return energia >= energiaMaxima; }
    void reiniciarEnergia() { energia = 0; }
    int obtenerEnergia() const { return energia; }

    void aumentarEnergia(int cantidad) {
        energia += cantidad;
        if (energia > energiaMaxima) energia = energiaMaxima;
    }

    void setListaEnemigos(QVector<Enemigo*>* lista);
    //void mantenerSalto(); //Se llama mientras W está presionado
private:
    QVector<QPixmap> framesCorrer;
    QVector<QPixmap> framesCaer;
    QVector<QPixmap> framesDisparo;
    QPixmap spriteCuerda;

    int frameActual;
    int contador;
    int velocidadAnimacion;
    bool disparando;

    void cargarAnimaciones();
    void actualizarSprite();
    bool enSuelo;

    QTimer* disparoTimer;
    int frameDisparoActual;

    void animarDisparoFrame(); // función que se llama cada 200ms

    int energia = 0;
    const int energiaMaxima = 100;

    QVector<Enemigo*>* listaEnemigos = nullptr;

};

#endif // GOKU_H
