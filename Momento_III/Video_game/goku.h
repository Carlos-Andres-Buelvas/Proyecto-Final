#ifndef GOKU_H
#define GOKU_H

#include "personaje.h"
#include <QVector>
#include <QPixmap>

class Goku : public Personaje {
    Q_OBJECT

public:
    Goku(float x, float y, float ancho, float alto);

    void mover() override;
    void saltar();
    void animarCorrer();
    void animarCaida();
    void animarDisparo();
    void animarCuerda();
    void acelerarCaida(); // tecla S
    bool estaEnSuelo() const { return enSuelo; }


private:
    QVector<QPixmap> framesCorrer;
    QVector<QPixmap> framesCaer;
    QVector<QPixmap> framesDisparo;
    QPixmap spriteCuerda;

    int frameActual;
    int contador;
    int velocidadAnimacion;

    void cargarAnimaciones();
    void actualizarSprite();
    bool enSuelo;

    QTimer* saltoTimer;
    int frameSaltoActual;

};

#endif // GOKU_H
